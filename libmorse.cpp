#include <cstring>
#include <limits>
#include "WProgram.h"
#include "libmorse.h"
#include "actions.h"
#include "prosign.h"

//---- Morse Code ----

int ditMicros(ditMicrosDefault);

void setDitMicros(int ms) {
  ditMicros = ms;
}

int getDitMicros() {
  return ditMicros;
}

// Initial frequency of the 'carrier wave' tones.
uint32_t txHz(A440);

uint32_t getTxHz() {
  return txHz;
}

void setTxHz(uint32_t Hz) {
  txHz = Hz;
}

//---- Copying the user ----

// The end of the user's input is determined by a timeout
// equal to two times the normal word gap.
elapsedMicros sinceTouch;

uint16_t leftCode(1);  	// Morse code reversed with sentinel.
uint8_t touchCount(0);

// Has it been at least 600 word gaps since the last activity?
bool isIdle() {
  return leftCode == 1 &&
    600 * wordGapFactor * getDitMicros() < sinceTouch;
}

bool letterExpired() {
  const int ImSlow(5);
  return ImSlow * getDitMicros() < sinceTouch;
}

// Has it been long enough that the word under construction
// should be considered complete?
bool wordExpired() {
  const int ImSlow(9);
  return ImSlow * getDitMicros() < sinceTouch;
}

// Accumulate a Morse token. If the token is in progress, return an
// invalid token. If the token is complete, return it. The caller is
// expected to call this function repeatedly until a token is
// completed or until the caller decides to stop for some other
// reason, e.g. timeout.
MorseToken getMorse() {
  // Make pins 0 and 1 function like an iambic keyer.
  // Since a dit or dah is echoed after it is keyed in, and during
  // this time the application is not listening or buffering, there's
  // no need for additional debouncing logic.
  bool ePin(touchPoll(ditPin));
  bool tPin(touchPoll(dahPin));
  
  if (ePin || tPin) {
    sinceTouch = 0;
    ++touchCount;
  }

  // Set the pitch to A#4 for user transmission.
  setTxHz(466);

  if (ePin && !tPin) { // Touching only E...
    txLetter('E');     // A single dit
    leftCode <<= 1;
  }
  
  if (!ePin && tPin) { // Touching only T...
    txLetter('T');     // A single dah
    leftCode = leftCode << 1 | 1;
  }
  
  if (ePin && tPin) { // Touching both E and T...
    if (leftCode & 1) {
      txLetter('E');     // A single dit
      leftCode <<= 1;
      ++touchCount;
    }
    txLetter('T');     // A single dah
    leftCode = leftCode << 1 | 1;
  }
  
  if (wordBits <= touchCount) {
    txError();
    touchCount = 0;
    leftCode = 1;
    sinceTouch = 0;
  }

  // Set the pitch to A4 for the system transmission.
  setTxHz(A440);

  // End of word timeout...
  if (leftCode != 1 && wordExpired()) {
    txTick(MorseToken());
    // Convert leftCode to rightCode by shifting and adding a sentinel
    // in the (new) left-most bit position.
    MorseToken
      rightCode((leftCode << 1 | 1) << (wordBits - touchCount - 1));

    touchCount = 0;
    leftCode = 1;
    return rightCode;
  } else if (touchCount == 0 && wordExpired()) {
    return MorseToken(1);
  } else {
    return MorseToken();
  }
}

//---- Command interpretation ----

// Small table of function pointers indexed by the bottom 8 bits of 
// our internal representation of a Morse code character or prosign.
//
// Actions are passed MorseToken instances so that a single action can
// handle several different tokens.
ActionFn actions[256];
ActionFn savedActions[256];

#define actionSize (sizeof actions/sizeof actions[0])

// Just your basic noop.
void doNothing(MorseToken) {}

// Load the table of actions with noops.
void initActions() {
  for (size_t i(0); i < actionSize; ++i) {
    actions[i] = doNothing;
  }
}

// Our internal representation of Morse code is left justified, that
// is to say, the high order bit is the first dit/dah. Most of the
// time, the bottom 8 bits will be zero, so create a hash by grabbing
// the top 8 bits.
uint8_t codeHash(MorseToken code) {
  return code >> (wordBits - CHAR_BIT);
}

// Hash a Morse code symbol to a table location and store action there.
void assignAction(MorseToken code, ActionFn action) {
  actions[codeHash(code)] = action;
}

// Invoke an action from the actions table.
void doAction(MorseToken code) {
  actions[codeHash(code)](code);
}

// A little safeguard against smashing the saved copy of the actions
// and not being able to get back to the primary commands.
bool restorePending(false);

// Save all the action pointers in preparation for overwriting some or
// all of them with action pointers for a mode.
// If the actions have been saved previously, the actions are not
// saved and the previously saved actions are not disturbed.
void saveActions() {
  if (!restorePending) {
    restorePending = true;
    for (size_t i(0); i < actionSize; ++i) {
      savedActions[i] = actions[i];
    }
  }
}

// If there are actions saved, restore them. Otherwise, transmit
// an error.
void restoreActions() {
  if (restorePending) {
    for (size_t i(0); i < actionSize; ++i) {
      actions[i] = savedActions[i];
    }
    restorePending = false;
  } else {
    txError();
  }
}

// Push the most recent symbol onto the stack, and then restore
// all the actions to their previous values.
void push1RestoreAll(MorseToken code) {
  pushSymbolStack(code);
  restoreActions();
}

void gradeRestore(MorseToken answer) {
  pushSymbolStack(answer);
  gradeSum(MorseToken());
  restoreActions();
}

//---- Conversion ----

#define MORSE_UNDEFINED_CHAR '^'

// Morse code for letters encoded as an implicit binary tree.
// Letters are written in breadth first order. Left child
// corresponds to dit; right child corresponds to dah.
// Underscore is used for undefined (or incomplete) codes.
// The root of the tree acts as a sentinel to simplify the process of
// converting the loop index into our left-justified code scheme.
//
// Here are some examples to show how it works:
//    000000  _           Not used
//    000001  _           Sentinel followed by zero bits
//    000010  E   .       Sentinel followed by a dit
//    000011  T   _       Sentinel followed by a dah
//    000100  I   . .     Sentinel followed by two dits
//    000101  A   . _     And so on...
//    000110  N   _ .
//    000111  M   _ _
//    001000  S   . . .
//    001001  U   . . _
//    001010  R   . _ .
//    001011  W   . _ _
//    001100  D   _ . .
//    001101  K   _ . _
//    001110  G   _ _ .
//    001111  O   _ _ _
const char morseTree[] =
  "^^"
  "ET"
  "IANM"
  "SURWDKGO"
  "HVF^L^PJBXCYZQ^^"
  "54^3^^^2^^+^^^^16=/^^^^^7^^^8^90"
  "^^^^^^^^^^^^?_^^^^\"^^.^^^^@^^^'^^-^^^^^^^^;!^(^^^^^,^^^^:^^^^^^^";

// Convert a Morse code into ASCII.
char m2a(MorseToken code) { 
  char result('x');
  int length(-2);
  uint8_t t(MORSE_MSB | (code >> 1));
  while (t) {
    t <<= 1;
    ++length;
  }
  uint8_t raw(code);
  raw >>= (CHAR_BIT * sizeof MorseToken(0) - length);
  raw |= 1 << length;
  if (raw < sizeof morseTree) {
    result = morseTree[raw];
  } else {
    // Translate the subset of the prosigns/abbreviations that can be
    // represented by a single ASCII character.
    switch (raw) {
    case MORSE_AA:     result = '\n'; break;
    case MORSE_AMPER:  result = '&'; break;
    case MORSE_APOSTR: result = '\''; break;
    case MORSE_AR:     result = '+'; break;
    case MORSE_AT:     result = '@'; break;
    case MORSE_BT:     result = '='; break;
    case MORSE_CLOSEP: result = ')'; break;
    case MORSE_COLON:  result = ':'; break;
    case MORSE_COMMA:  result = ','; break;
    case MORSE_DOLLAR: result = '$'; break;
    case MORSE_ERROR:  result = 'x'; break;
    case MORSE_EXCLAI: result = '!'; break;
    case MORSE_INTER:  result = '?'; break;
    case MORSE_MINUS:  result = '-'; break;
    case MORSE_OK:     result = 'K'; break; // Literally, K.
    case MORSE_OPENP:  result = '('; break;
    case MORSE_PERIOD: result = '.'; break;
    case MORSE_QUOTE:  result = '"'; break;
    case MORSE_SEMI:   result = ';'; break;
    case MORSE_SLASH:  result = '/'; break;
    case MORSE_UNDER:  result = '_'; break;
    default:
      ; // Nothing more to do since result is initialized to 'x'.
    }
  }
  return result;
}

// Convert a single ASCII character into a Morse code token.
MorseToken a2m(char a) {
  uint8_t letterCode(MORSE_ERROR); // XXX doesn't fit in a byte
  ptrdiff_t key(strchr(morseTree, a) - morseTree);
  if (0 < key && morseTree[key] != MORSE_UNDEFINED_CHAR) {
    // Add the sentinel to the right.
    letterCode = key << 1 | 1;
    while (!(letterCode & MORSE_MSB)) {
      letterCode <<= 1;
    }
    letterCode <<= 1;
  }
  return MorseToken(letterCode);
}

// The given Morse code token should be a digit. Return the
// corresponding number. If the input is not a digit, transmits the
// error prosign and returns an undefined value.
uint8_t m2i(MorseToken m) {
  uint8_t result(m2a(m) - '0');
  if (9 < result) {
    txError();
  }
  return result;
}

// Given an ASCII string representing a prosign, return a 
// code composed by concatenating the morse code for each 
// character in the prosign without inter-symbol gaps.
//
// For example "AB" yields
// 
//    A    . _           01100000
//    B        _ . . .     10001000
//    AB   . _ _ . . .   01100010
//
// Likewise "ABC" yields 0110001101010000.
//
// If the translation is longer than will fit in MorseToken, the return
// value will be the longest suffix of the input that does fit. All
// the prosigns I'm aware of fit without truncation if MorseToken is 16
// bits or more.
MorseToken a2p(char* prosign) {
  uint8_t letterCode(a2m(prosign[0]));
  uint8_t t(letterCode);
  size_t length(0);
  while (t) { ++length; t <<= 1; }
  char* p(prosign + 1);
  while (*p) {
    char a(*p);
    uint8_t m(a2m(a));
    // Erase the sentinel (always 1).
    const size_t wordBits(CHAR_BIT * sizeof MorseToken(0));
    letterCode ^= 1 << (wordBits - length);
    // Shift the new symbol and or it in after the previous.
    letterCode |= m >> (length - 1);
    // Update the length.
    while (m) { ++length; m <<= 1; }
    ++p;
  }
  return MorseToken(letterCode);
}

//---- Transmission ----

// Transmit a digit in Morse code, for memory-limited applications.
// N.B. The digit is the actual digit 0-9, not ASCII.
// N.B. This is a standalone function. It does not use the
//      left-justified encoding scheme described in the header file.
// This routine takes 80 bytes.
//
// Morse code for the digits:
//    0: -----    1: .----    2: ..---    3: ...--    4: ....-
//    5: .....    6: -....    7: --...    8: ---..    9: ----.
void blinkMorseDigit(uint8_t digit) {
  signed char b111110(0x3E);
  const signed char dend(digit - 5);
  for (signed char i(digit); i != dend; --i) {
    beep(440,
	 ditMicros * (b111110 >> i & 1 ? ditFactor : dahFactor));
    delayMicroseconds(ditMicros *
		      (i - 1 == dend ? letterGapFactor : wordGapFactor));
  }
}

// Blink a single Morse code digit.
// Implemented with a table lookup.
// 96 bytes
void bmd096(uint8_t digit) {
  uint8_t t[] = {
    B8(111111), B8(111110), B8(111100), B8(111000), B8(110000),
    B8(100000), B8(100001), B8(100011), B8(100111), B8(101111)
  };
  uint8_t m(t[digit]);
  while (m != 1) {
    beep(440, ditMicros * (m & 1 ? dahFactor : ditFactor));
    delayMicroseconds(ditMicros * (m != 1 ? letterGapFactor : wordGapFactor));
    m >>= 1;
  }
}

// Blink a single Morse code digit.
// Implemented implemented with a table lookup and the observation
// that Morse(x) == ~Morse(x + 5) when x < 5.
// 104 bytes
void bmd104(uint8_t digit) {
  uint8_t t[] = { B8(11111), B8(11110), B8(11100), B8(11000), B8(10000) };
  uint8_t m(digit < 6 ? t[digit] : t[digit - 5] ^ B8(11111));
  m |= 1 << 5;
  while (m != 1) {
    beep(440, ditMicros * (m & 1 ? dahFactor : ditFactor));
    delayMicroseconds(ditMicros *
		      (m != 1 ? letterGapFactor : wordGapFactor));
    m >>= 1;
  }
}

// Transmit a Morse code prosign indicating an error.
void txError() {
  Serial.print("Error [........] ");
  txString("EEEEEEEE");
}

typedef void (* SymbolFn)(bool);

void foreachSymbol(MorseToken code, SymbolFn fn) {
  for (uint8_t raw(code); raw != MORSE_MSB; raw <<= 1) {
    fn(raw & MORSE_MSB);
  }
}

// Return a string of dots and dashes corresponding to code.
// XXX change declaration so compiler prevents user from using
// XXX the return value to smash the static buffer.
char* m2didah(MorseToken code) {
  static char result[14];
  uint8_t raw(code);
  uint8_t i(0);
  while (raw != MORSE_MSB) {
    result[i] = raw & MORSE_MSB ? '_' : '.';
    raw <<= 1;
    ++i;
  }
  result[i] = '\000';
  return result;
}

// Transmit a Morse code symbol. The parameter is encoded as a left-justified
// bit sequence where 0 represents dit and 1 represents dah. Since Morse code
// symbols vary in length, the code is terminated with a 1 after the right-
// most token. 
void txLiteral(MorseToken code) {
  if (code.valid()) {		// Guard against an infinite loop.
    uint8_t raw(code);
    while (raw != MORSE_MSB) {
      beep(txHz, ditMicros * (raw & MORSE_MSB ? dahFactor : ditFactor));
      Serial.print(raw & MORSE_MSB ? "_" : ".");
      delayMicroseconds(ditMicros);
      raw <<= 1;
    }
    Serial.print(' ');
    delayMicroseconds((letterGapFactor - ditFactor) * ditMicros);
  } else {
    txError();
  }
}

// Transmit a Morse code symbol consisting of one letter or number. The 
// ASCII-to-Morse translation table is represented as an implicit binary
// tree. See the header file for details.
//
// If x is not in the set [A-Z0-9], transmit an 'error' prosign.
void txLetter(char a) {
  MorseToken letterCode(a2m(a));
  if (letterCode != MORSE_ERROR) {
    txLiteral(letterCode);
  } else {
    txError();
    Serial.println(a);
  }
}

// Transmit an unsigned integer in Morse code.
void txUnsigned(uint16_t n) {
  // The constant digits10 below is not the maximum number of digits
  // required to represent the parameter type. Rather, it is the
  // maximum number of digits that can be represented in a variable of
  // the given type without change. E.g. the type char can represent
  // 99, but not 999, so digits10 is 2, not 3, hence we add 1 to store
  // all the digits that could be needed for input argument n.
  char digitBuffer[std::numeric_limits<uint16_t>::digits10 + 1];
  uint16_t digitCount(0);
  do {
    // Convert the least significant digit to ASCII.
    digitBuffer[digitCount++] = '0' + n % 10;
    // Proceed to the next digit.
    n /= 10;
  } while (n);
  while (digitCount) {
    txLetter(digitBuffer[--digitCount]);
    Serial.print(digitBuffer[digitCount]);
  }
  Serial.println("");
}

// Transmit a Morse code string. Prosigns are delimited by
// underscores (^), e.g. "^QTR^ 2112".
//
// Abort if the dit key is touched.
void txString(const char* s) {
  if (*s) {
    for (char* p(const_cast<char*>(s));
	 *p && !touchPoll(ditPin);
	 ++p)
    {
      if (*p == ' ') {
	Serial.print(' ');
	delayMicroseconds(wordGapFactor * ditMicros);
      } else if (*p == '^') {
	// Accumulate the following prosign and transmit it.
	char qbuf[5] = { '1', '2', '3', '4', '\0' };
	char* q(qbuf);
	while (*q && *++p != '^') *q++ = *p;
	*q = 0;
	txLiteral(a2p(qbuf));
      } else {
	txLetter(*p);
      }
    }
    delayMicroseconds((wordGapFactor - letterGapFactor) * ditMicros);
    Serial.println();
    Serial.println(s);
  }
}

//---- Pattern Matching ----

// Literals - A-Z, 0-9, punctuation
// Prosigns - ^CQ^
// Word pause - space
// Digit - d
// Any character - c
// Letter - a
// Word - w
// Number - n
//
// How do we indicate one or more?
// Maybe d is exactly one digit
// and dd is any number?
//
// "QTR?" - What is the correct time?
// "dd QRQ" - Send faster at (dd words per minute)
// "dd QRS" - Send slower at (dd words per minute)
// "QRZ?" - By whom am I being called?
// "w QRZ" - You are being called by (w)
// "QRU?" - Do you have anything for me? (What is the next alarm message?)
// "QRU" - I have nothing.
// "QSD" - Your keying is incorrect; your signals are bad.
// "d QSG" - Transmit (d) telegrams at a time.
// "QSL?" - Can you acknowledge receipt
// "QSL" - Acknowledging receipt
// "QSM?" - Shall I repeat the last telegram?
// "QSM" - Repeat the last telegram.
// "d QSZ" - Send each word or group (d) times
// "dd QTC" - I have (d) telegrams to send.
// "QTH?" - What is your position?
// "dd QTA" - Erase telegram (dd)
// "QTB?" - Do you agree with my word count?
// "QTB" - I do not agree; I shall repeat the first letter or first figure
//         of each

// Score the given pattern against the state of the symbol stack. The
// maximum score is 255 and decreases with increased distance from the
// pattern. 
uint8_t matchScore(const char* pattern) {
  uint8_t score(255);
  const char* p(pattern);
  while (*p) ++p;
  size_t len(p - pattern);
  if (symbolStackSize() < len) {
    score = 0;
  } else {
    //XXX assumes score is decremented by at most 1 in the loop
    for (uint8_t i(0); len && score; --len, ++i) {
      char c(*(pattern + i));
      char si(m2a(s(len - 1)));
      switch (c) {
      case 'a':
	// Match any
	break;
      case 'c':
	// Match a character
	break;
      case 'd':
	// Match a digit
	score -= score && (9 < si - '0');
	break;
      case 'n':
	// Match one or more digits
	break;
      case 'w':
	// Match a word
	break;
      case 's':
	// Match one or more words, terminated by
	break;
      case '^':
	// Match a prosign.
	break;
      default:
	// Match the literal value of c.
	score -= score && si != c;
      };
    }
  }
  return score;
}

void keyboardMode(MorseToken) {
  txString("KB");
  bool didSpace(true);
  elapsedMicros sinceKeyboard(0);
  for (MorseToken m(getMorse()); m != a2p("AR"); m = getMorse()) {
    if (m.valid()) {
      if (m == a2p("BT")) {
	Keyboard.print("\n");
	Serial.print("\n");
	didSpace = true;
      } else if (m == a2p("AL")) {
	Keyboard.print("\n\n");
	Serial.print("\n\n");
	didSpace = true;
      } else {
	Keyboard.print(m2a(m));
	Serial.print(m2a(m));
	didSpace = false;
      }
      sinceKeyboard = 0;
    } else if (uint8_t(m) == 1 &&
	       (20 * getDitMicros() < sinceKeyboard) && !didSpace)
    {
      txTock(MorseToken());
      Keyboard.print(' ');
      Serial.print(' ');
      didSpace = true;
    }
  }
  txString("?");
}

void setVolume(MorseToken) {
  size_t n(symbolStackSize());
  size_t dutyCycle(0);
  (2 < n) && (dutyCycle += 100 * m2i(s(2)));
  (1 < n) && (dutyCycle +=  10 * m2i(s(1)));
  (0 < n) && (dutyCycle +=   1 * m2i(s(0)));
  popN(3 - (2 < n) - (1 < n));
  setDuty(dutyCycle);
}
