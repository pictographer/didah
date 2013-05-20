#include "WProgram.h"
#include <EEPROM.h>
#include "actions.h"
#include "libmorse.h"
#include "mac.h"
#include "notes.h"
#include "prosign.h"
#include "stack.h"

// Transmit the 24-hr time.
void txTimeHHMM(MorseToken code) {
  //            0         1         2
  //            012345678901234567890
  char buf[] = "QRT QRT HH MM ? HH MM";
  unsigned long secs(Teensy3Clock.get());
  unsigned int m((secs / 60) % 60);
  unsigned int h((secs / 3600) % 24);
  buf[8]  = '0' + h / 10;
  buf[9]  = '0' + h % 10;
  buf[11] = '0' + m / 10;
  buf[12] = '0' + m % 10;
  buf[16] = buf[8];
  buf[17] = buf[9];
  buf[19] = buf[11];
  buf[20] = buf[12];
  txString(buf);
}

// Transmit the internal temperature in integer Celcius.
//XXX Not calibrated!
void txInternalTemperature(MorseToken) {
  double a(analogRead(38));
  int degC(414.7 - a/35.7);
  txUnsigned(degC);
  Serial.println(degC);
}

// Transmit the mac address.
void txMacAddress(MorseToken) {
  read_mac();
  print_mac();
  for (uint8_t* p(mac); p < mac + 6; ++p) {
    uint8_t octet1(*p >> 4);
    uint8_t octet2(*p && 0x0F);
    txLetter("0123456789ABCDEF"[octet1]);
    txLetter("0123456789ABCDEF"[octet2]);
    delay(200);
  }
}

// Play with the PWM frequency by transmitting a chromatic scale or
// something like that.
void playScale(MorseToken) {
  for (size_t i(24); i < sizeof(noteHz)/sizeof(noteHz[0]); i += 1) {
    analogWriteFrequency(beepPin, noteHz[i]+0.5);
    analogWrite(beepPin, duty50);
    delayMicroseconds(getDitMicros());
    analogWrite(beepPin, 0);
  }
}

// Play a PWM tone at a given frequency and duration.
void playTone(size_t frequency, size_t duration) {
  analogWriteFrequency(beepPin, frequency);
  analogWrite(beepPin, duty50);
  delayMicroseconds(duration);
  analogWrite(beepPin, 0);
  delayMicroseconds(duration);
}

// Play a hiss like an old phonograph record.
void phonographHiss(int duration) {
  elapsedMicros t(0);
  while (t < duration) {
    digitalWrite(beepPin, HIGH);
    delayMicroseconds((1 + random(100)) * 1136);
    digitalWrite(beepPin, LOW);
    delayMicroseconds(100);
  }
}

// Play a whooping sound like a toy ray gun.
void rayGun(int duration) {
  elapsedMicros t(0);
  while (t < duration) {
    for (int i(1); i < 25; ++i) {
      digitalWrite(beepPin, HIGH);
      delayMicroseconds(i * 1136/12);
      digitalWrite(beepPin, LOW);
      delayMicroseconds(i * 1136/12);
    }
  }
}

// Shuffling cash like a bill counter
void shufflingCash(int duration) {
  elapsedMicros t(0);
  while (t < duration) {
    for (int i(1); i < 25; ++i) {
      digitalWrite(beepPin, HIGH);
      delayMicroseconds((1 + random(i)) * 1136/12);
      digitalWrite(beepPin, LOW);
      delayMicroseconds((1 + random(i)) * 1136/12);
    }
  }
}

// Play a series of tones randomly bounded by the reading of the dit
// pin.
void pressureTone(int duration) {
  elapsedMicros t(0);
  while (t < duration) {
    for (int i(1); i < 50; ++i) {
      digitalWrite(beepPin, HIGH);
      delayMicroseconds((500 + random(log(touchRead(ditPin)))));
      digitalWrite(beepPin, LOW);
      delayMicroseconds((500 + random(log(touchRead(ditPin)))));
    }
  }
}

// Play one of 10 sound effects based on the digit on the stack.
void playSoundEffect(MorseToken code) {
  analogWriteFrequency(beepPin, noteHz[48]+0.5);
  analogWrite(beepPin, duty50);
  delayMicroseconds(getDitMicros());
  analogWrite(beepPin, 0);
  if (symbolStackSize()) {
    uint8_t n(m2i(s(0)));
    elapsedMicros t;
    size_t steps[] = { 0, 2, 3, 5, 7, 8, 10 };
    switch (n) {
    case 0:
      t = 0;
      // Low pulsing static rumble
      while (t < 60 * ditMicrosDefault) {
  for (int i(1); i < 25; ++i) {
	  digitalWrite(beepPin, HIGH);
	  delayMicroseconds((1 + random(i)) * 1136);
	  digitalWrite(beepPin, LOW);
	  delayMicroseconds((1 + random(i)) * 1136);
	}
      }
      break;
    case 1:
      shufflingCash(120 * ditMicrosDefault);
      break;
    case 2:
      t = 0;
      // Descending beats
      while (t < 120 * ditMicrosDefault) {
	for (int i(1); i < 75; ++i) {
	  digitalWrite(beepPin, HIGH);
	  delayMicroseconds(i * 1136/12);
	  digitalWrite(beepPin, LOW);
	  delayMicroseconds(i * 1136/12);
	}
      }
      break;
    case 3:
      t = 0;
      // Buzzer
      while (t < 60 * ditMicrosDefault) {
	playTone(noteHz[36], ditMicrosDefault/30);
	playTone(noteHz[48], ditMicrosDefault/30);
	playTone(noteHz[52], ditMicrosDefault/60);
	playTone(noteHz[57], ditMicrosDefault/60);
      }
      break;
    case 4:
      t = 0;
      // Humm, kinda like a machine hum.
      while (t < 60 * ditMicrosDefault) {
	for (int i(0); i < 49; ++i) {
	  playTone(noteHz[i], ditMicrosDefault/10);
	}
      }
      break;
    case 5:
      t = 0;
      // ascending and repeating 
      while (t < 60 * ditMicrosDefault) {
	for (int i(0); i < 49; ++i) {
	  playTone(noteHz[i], ditMicrosDefault/2);
	}
      }
      break;
    case 6:
      t = 0;
      // ascending more quickly
      while (t < 60 * ditMicrosDefault) {
	for (int i(0); i < 49; ++i) {
	  playTone(noteHz[random(1,96)], ditMicrosDefault/10);
	}
      }
      break;
    case 7:
      t = 0;
      // ?
      while (t < 60 * ditMicrosDefault) {
	for (int i(0); i < 7; ++i) {
	  playTone(noteHz[(24 + steps[i % 7]) % 97], ditMicrosDefault);
	}
      }
      break;
    case 8:
      t = 0;
      // Almost white noise, like water from a tap
      while (t < 60 * ditMicrosDefault) {
	digitalWrite(beepPin, HIGH);
	delayMicroseconds(random(100,1000));
	digitalWrite(beepPin, LOW);
	delayMicroseconds(random(100,1000));
      }
      break;
    case 9:
      t = 0;
      // Almost white noise, like water from a tap
      while (t < 60 * ditMicrosDefault) {
	digitalWrite(beepPin, HIGH);
	delayMicroseconds(random(10,8000));
	digitalWrite(beepPin, LOW);
	delayMicroseconds(random(10,8000));
      }
      t = 0;
      // Sort of like a pinball machine
      { int i(2);
	while (t < 60 * ditMicrosDefault) {
	  for (int k(0); k < 10; ++k) {
	    digitalWrite(beepPin, HIGH);
	    delayMicroseconds(i);
	    digitalWrite(beepPin, LOW);
	    delayMicroseconds(i);
	  }
	  i <<= 2;
	  if (4000 < i) { i = 2; }
	}
      }
    }
    txString("K");
  } else {
    txError();
  }
}

// If the top two elements of the stack are digits, convert them to a
// number between 0-99 inclusive. Play the corresponding note.
void playNote(MorseToken) {
  if (1 < symbolStackSize() && isdigit(m2a(s(0))) && isdigit(m2a(s(1)))) {
    size_t note(10 * (m2a(s(1)) - '0') + (m2a(s(0)) - '0'));
    playTone(noteHz[note] + 0.5, getDitMicros() * 60);
    txUnsigned(note);
  } else {
    txError();
  }
}

// Return the duration of the Morse code for the given character in
// dits. E.g. an E would return 2; and an N would return 6.
uint8_t durationDits(char a) {
  if (a == ' ') {
    return wordGapFactor;
  } else {
    uint8_t code(a2m(a));
    uint8_t count(0);
    while (code != MORSE_MSB) {
      count += ((code & MORSE_MSB) ? dahFactor : ditFactor) + ditFactor;
      code <<= 1;
    }
    return count;
  }
}

// Play the A.B.C's with each single syllable letter replaced by the
// corresponding Morse code. W is transmitted once for each syllable.
// After Z, the remainder of the song is played as simple tones.
void txAlphabetSong(MorseToken) {
  uint32_t saveHz(getTxHz());
  int saveDitMicros(getDitMicros());
  const char* letters("ABCDEFGHIJKLMNOPQRSTUVWWWXY Z ");
  const char*
    notes("C42C42G42G42" "A52A52G44" "F42F42E42E42" "D41D41D41D41C44"
	  "G42G42F44"    "E42E42D44" "G41G41G42F44" "E42E42D44"
	  "C42C42G42G42" "A52A52G44" "F42F42E42E42" "D42D42C44");
  char* l(const_cast<char*>(letters));
  char* p(const_cast<char*>(notes));
  while (*p) {
    size_t steps[] = { 0, 2, 3, 5, 7, 8, 10 };
    size_t note(steps[p[0] - 'A'] + 12 * (p[1]-'0'));
    size_t frequency(noteHz[note] + 0.5);
    size_t duration(saveDitMicros * (p[2] - '0') * 2 / 3);
    char buf[] = { '\0', ' ', '\0' };
    buf[0] = l[0];
    if (l[1]) {
      setTxHz(frequency);
      setDitMicros(duration);
      txString(buf);
      delayMicroseconds((14 * saveDitMicros + 1) -
			(saveDitMicros * durationDits(l[0])));
      ++l;
    } else {
      // This part goes much faster, but it doesn't convey much
      // information.
      playTone(frequency, duration);
    }
    p += 3;
  }
  setTxHz(saveHz);
  setDitMicros(saveDitMicros);
}

// Read a byte from the prom. (Proof of concept.)
void txProm(MorseToken) {
  MorseToken byte(EEPROM.read(0));
  Serial.print("EEPROM[0] = ");
  Serial.println(uint8_t(byte));
  txLiteral(byte);
}

// The symbol stack is a buffer for user input which does not
// translate directly into single-symbol command.
Stack<MorseToken, 256> tokenStack;

// Report the stack size. This function provides a little insulation
// so that we can change the implementation.
size_t symbolStackSize() {
  return tokenStack.Size();
}

// Access the stack using an index where 0 is the top of the stack.
MorseToken s(size_t i) {
  MorseToken result(MORSE_ERROR);//xxx only if 1 < sizeof MorseToken 
  if (i < symbolStackSize()) {
    result = tokenStack.S(i);
  }
  return result;
}

// Push a symbol on to the symbol stack. If the stack is already full,
// simply overwrite the top of the stack.
void pushSymbolStack(MorseToken code) {
  tokenStack.Push(code);
}

// Discard the top element of the stack unless the stack is empty in
// which case do nothing.
void popSymbolStack(MorseToken) {
  tokenStack.Pop();
}

// Swap s0 and s1.
void exchangeStack(MorseToken) {
  if (1 < tokenStack.Size()) {
    tokenStack.Exchange();
  } else {
    txError();
  }
}

// Discard all elements of the stack.
void clearSymbolStack(MorseToken) {
  tokenStack.Clear();
}

// Transmit the top element of the stack.
void txSymbolStackTop(MorseToken) {
  //  txLiteral(symbolStack[symbolStackTop - 1]);
  txLiteral(tokenStack.S(0));
}

// List the contents of the symbol stack.
void txListSymbolStack(MorseToken) {
  for (size_t i(0); i < tokenStack.Size(); ++i) {
    txLiteral(tokenStack.S(i));
    Serial.println(m2a(tokenStack.S(i)));
  }
}

// Pop the stack n times or until it is empty.
void popN(uint8_t n) {
  uint8_t i(n);
  while (i-- && tokenStack.Size()) {
    tokenStack.Pop();
  }
}

// Write the top element of the stack to address 0 of EEPROM.
void setProm(MorseToken) {
  if (0 < symbolStackSize()) {
    EEPROM.write(0, s(0));
    txLiteral(MorseToken(MORSE_ACK));
  } else {
    txError();
  }
}

// Transmit a question mark to prompt for input.
void txPrompt(MorseToken code) {
  Serial.println("?");
  txLiteral(MorseToken(MORSE_INTER));
}

// Transmit the digits from 0 to 9.
void tx0to9(MorseToken) {
  //txString("0 1 2 3 4 5 6 7 8 9 ?");
  long x(random(9));
  txLiteral(a2m(x + '0'));
}

// Histogram of digits for which the user made a mistake
uint8_t digitErr[10];

// Start the error count at the maximum index of digitErr[] so that a
// random ordinal in the range (0..digitalErrorCount) covers
// digitErr[].
uint16_t digitErrCount(9);

// Generate a random digit with the probability distribution implied
// by the digitErr histogram. 
//
// The sum of the entries in digitErr always equals digitErrCount. A
// random ordinal number in the range (0..digitErrCount) is compared
// to the cumulative distribution (calculated dynamically from the
// ordinary histogram. As long as the random number is less than the
// cumulative distribution, the search continues.
uint8_t getDigitByWeight() {
  uint8_t a(random(digitErrCount));
  uint8_t result(9);
  for (uint8_t i(0), sum(0); i < 10; ++i) {
    sum += digitErr[i] + 1;
    if (a < sum) {
      result = i;
      break;
    }
  }
  return result;
}

// Present a simple challenge consisting of two random digits to 
// add. The user tries to enter the last digit of their sum.
void askSum(MorseToken) {
  uint8_t a(getDigitByWeight());
  uint8_t b(getDigitByWeight());
  char buf[3] = { a + '0', b + '0', 0 };
  txString(buf);
  pushSymbolStack(a2m(buf[0]));
  pushSymbolStack(a2m(buf[1]));
  saveActions();
  for (size_t i('0'); i <= '9' ; ++i) {
    assignAction(a2m(i), gradeRestore);
  }
}

// The top three entries on the stack are two digits to add and their
// proported sum mod 10. Transmit Y/N to indicate whether or not the
// sum is correct. If the sum is not correct, transmit the correct
// sum. Update the weights to bias future random number selection.
void gradeSum(MorseToken) {
  if (2 < symbolStackSize()) {
    uint8_t s1(m2i(s(1)));
    uint8_t s2(m2i(s(2)));
    uint8_t s0(m2a(s(0)));
    uint8_t sum((s1 + s2) % 10);
    Serial.print(s2);
    Serial.print(" ");
    Serial.print(s1);
    Serial.print(" ");
    Serial.print(s0);
    Serial.print(" sum: ");
    Serial.println(sum);
    bool correct(m2i(s(0)) == sum);
    if (correct) {
      txString("Y");
      // Decrease the error counts associated with the digits the user
      // just added incorrectly, taking care not to underflow.
      uint8_t d1(!!digitErr[s1]);
      uint8_t d2(!!digitErr[s2]);
      digitErr[s1] -= d1; 
      digitErr[s2] -= d2;
      digitErrCount -= (d1 + d2);
      Serial.print("Decrementing ");
      Serial.print(s1);
      Serial.print(". New weight is: ");
      Serial.println(digitErr[s1]);
      Serial.print("Decrementing ");
      Serial.print(s2);
      Serial.print(". New weight is: ");
      Serial.println(digitErr[s2]);
    } else {
      //            012345678901
      char buf[] = "N  a + b = c";
      buf[3] = s2 + '0';
      buf[7] = s1 + '0';
      buf[11] = sum + '0';
      txString(buf);
      // Increase the error counts associated with the digits the user
      // just added incorrectly, taking care not to overflow.
      uint8_t d1(3 * (digitErr[s1] - 3 < 255));
      uint8_t d2(3 * (digitErr[s2] - 3 < 255));
      digitErr[s1] += d1;
      digitErr[s2] += d2;
      digitErrCount += d1 + d2;
      Serial.print("Incrementing ");
      Serial.print(s1);
      Serial.print(". New weight is: ");
      Serial.println(digitErr[s1]);
      Serial.print("Incrementing ");
      Serial.print(s2);
      Serial.print(". New weight is: ");
      Serial.println(digitErr[s2]);
    }
    // If the user plays long enough without a reset to saturate two
    // of the digits chosen at random, divide all the error counts
    // by 2.
    if (digitErr[s1] == 255 && digitErr[s2] == 255) {
      for (uint8_t i(0); i < 10; ++i) {
	digitErr[i] <<= 1;
      }
      digitErrCount <<= 1;
      Serial.println("Halving all weights.");
    }
    Serial.print("Digit error count is: ");
    Serial.println(digitErrCount);
    popN(3);
  } else {
    txError();
  }
}

// Prompt with a random letter of the alphabet. The user enters the
// letter then enters I to get graded.
void askNext(MorseToken) {
  uint8_t a(random(26));
  char buf[3] = { a + 'A', 0 };
  txString(buf);
  uint8_t b((a + 1) % 26);
  pushSymbolStack(a2m(b + 'A'));
  assignAction(a2m('I'), gradeNext);
  saveActions();
  for (size_t i('A'); i <= 'Z' ; ++i) {
    assignAction(a2m(i), push1RestoreAll);
  }
}

void gradeNext(MorseToken) {
  if (1 < symbolStackSize()) {
    if (s(1) == s(0)) {
      txString("Y");
      popSymbolStack(MorseToken());
      popSymbolStack(MorseToken());
    } else {
      txString("N");
      popSymbolStack(MorseToken());
      txSymbolStackTop(MorseToken());
      popSymbolStack(MorseToken());
    }
  } else {
    txError();
  }
}

// Set the time according to the last four digits keyed in.
void setTime(MorseToken) {
  if (3 < symbolStackSize()) {
    Teensy3Clock.set(60 *
		     (60 * (10 * (m2a(s(3)) - '0') + (m2a(s(2)) - '0')) +
			   (10 * (m2a(s(1)) - '0') + (m2a(s(0)) - '0'))));
    Serial.print("Setting clock to ");
  } else {
    txError();
  }
  txTimeHHMM(MorseToken());
}

//XXX obsolete
char qbuf[4] = { 'x', 'x', 'x', 'x' };

//XXX obsolete
void clearQCode() {
  for (size_t i(0); i < 4; ++i) qbuf[i] = 'x';
}

void slowDown() {
  Serial.println("QRS - slower");
  clearQCode();
  txString("QRS ");
  int d(getDitMicros());
  setDitMicros(d + 5000);
  txString("QRS");
}

void speedUp() {
  Serial.println("QRQ - faster");
  clearQCode();
  txString("QRQ ");
  int d(getDitMicros());
  setDitMicros(d - 5000);
  txString("QRQ");
}

// Process a few Q codes.
//    QRS - slow down
//    QRQ - speed up
//    QTR? - what is the time
//    QSR - please repeat your call (XXX Would QSM be better?)
void dispatchQCode(MorseToken code) {
  pushSymbolStack(code);
  if (matchScore("QRS") == 255) {
    slowDown();
    popN(3);		   // XXX maybe need to build into matchScore?
  } else if (matchScore("QRQ") == 255) {
    speedUp();
    popN(3);
  } else if (matchScore("QTR?") == 255) {
    txTimeHHMM(MorseToken());
    popN(4);
  } else if (matchScore("ddddQTR") == 255) {
    popN(3);
    setTime(MorseToken());
    popN(4);
  } else if (matchScore("QSR") == 255) {
    txString("ET1");
  } else if (matchScore("ddddQRX") == 255) {
    Serial.println("Set time");
    // Set an alarm for HH:MM.
    popN(3);
    setAlarm(MorseToken());
  } else if (matchScore("X") == 255) {
    exchangeStack(MorseToken());
  } else if (matchScore("QRX?") == 255) {
    // Report alarm time or N for no alarm.
  } else if (matchScore("dQSM") == 255) {
    // Repeat telegram d
    //XXX an M wound't get here because it isn't assigned this action.
  } else if (matchScore("dQTA") == 255) {
    // Cancel telegram d
  } else if (matchScore("dQTC") == 255) {
    // Await d telegrams
    // dQRJ - Await d words
    // _CT_ means 'start copying'
    // _AR_ or + means 'end of message'
  } else if (matchScore("QTC?") == 255) {
    // Report number of stored telegrams
  } else if (matchScore("ABC") == 255) {
    txAlphabetSong(MorseToken());
  } else if (matchScore("C") == 255) {
    clearSymbolStack(MorseToken());
  } else if (matchScore("ddA") == 255) {
    txAlfaBravoCharlie(MorseToken());
  }
}

void txAR(MorseToken) {
  txString("^AR^ ^AR^ ^AR^");
}

void txEcho(MorseToken code) {
  txString("II"); // I say again.
  txLiteral(code);
  txString(" ^AR^"); // End of message.
}

void txGreeting(MorseToken code) {
  txString("CQ CQ CQ DE ET1 ET1 K");
  Serial.println();
}

void getPressure(MorseToken code) {
  txString("");
  analogWriteFrequency(beepPin, 0);
  analogWrite(beepPin, duty50);
  while (!touchPoll(dahPin)) {
    uint32_t ci(touchRead(ditPin));
    Serial.println(ci);
    uint32_t f(map(ci, 500, 65535, 33, 4186));
    analogWriteFrequency(beepPin, f);
    analogWrite(beepPin, duty50);
    delayMicroseconds(ditMicrosDefault);
  }
  txString("^AR^");
}

void txTick(MorseToken) {
  playTone(noteHz[96], ditMicrosDefault/2);
}

void txTiTick(MorseToken) {
  playTone(noteHz[98], ditMicrosDefault/4);
  delayMicroseconds(ditMicrosDefault/2);
  playTone(noteHz[98], ditMicrosDefault/4);
}

void txTock(MorseToken) {
  playTone(noteHz[84], ditMicrosDefault/2);
}

const char* alphaBravoCharlie("ALFA BRAVO CHARLIE DELTA "
			      "ECHO FOXTROT GOLF HOTEL "
			      "INDIA JULIETT KILO LIMA "
			      "MIKE NOVEMBER OSCAR PAPA "
			      "QUEBEC ROMEO SIERRA TANGO "
			      "UNIFORM VICTOR WHISKEY "
			      "X-RAY YANKEE ZULU ");

void txAlfaBravoCharlie(MorseToken code) {
  //  uint8_t n(m2a(code) - 'A');
  uint8_t n(0);
  if (1 < symbolStackSize()) {
    n = 10* m2i(s(1)) + m2i(s(0));
    popN(2);
  } else if (symbolStackSize()) {
    n = m2i(s(0));
    popN(1);
  }
  if (0 <= n && n <= ('Z' - 'A')) {
    char letterName[1 + sizeof "NOVEMBER"];
    const char* p(alphaBravoCharlie);
    size_t i(0);
    while (*p) {
      if (*p == ' ') {
	if (n) {
	  --n;
	} else {
	  break;
	}
      } else if (!n) {
	letterName[i++] = *p;
      }
      ++p;
    }
    letterName[i] = '\000';
    txString(letterName);
  } else {
    txError();
    Serial.println("txAlfaBravoCharlie: Range error.");
  }
}

// This is a little harder than askNext in that the extra letters can
// be distracting, but it is a little easier because there's more
// context. I expect that for beginners, the former is easier, but for
// intermediates, the latter is good practice.
void askNextAlfa(MorseToken) {
  uint8_t a(random(26));
  txAlfaBravoCharlie(a2m(a + 'A'));
  uint8_t b((a + 1) % 26);
  pushSymbolStack(a2m(b + 'A'));
  assignAction(a2m('I'), gradeNext);
  saveActions();
  for (size_t i('A'); i <= 'Z' ; ++i) {
    assignAction(a2m(i), push1RestoreAll);
  }
}

// Just like txAlfaBravoCharlie, but adds digits from
// the stack to expose the entire alphabet.
void txAlphaNNN(MorseToken) {
  uint8_t n(0);
  uint8_t k(symbolStackSize());
  if (0 < k) {
    n += m2i(s(0));
    popSymbolStack(MorseToken());
  }
  if (1 < k) {
    n += 10 * m2i(s(1));
    popSymbolStack(MorseToken());
  }
  if (0 <= n && n <= ('Z' - 'A')) {
    char letterName[1 + sizeof "NOVEMBER"];
    const char* p(alphaBravoCharlie);
    size_t i(0);
    while (*p) {
      if (*p == ' ') {
	if (n) {
	  --n;
	} else {
	  break;
	}
      } else if (!n) {
	letterName[i++] = *p;
      }
      ++p;
    }
    txString(letterName);
  } else {
    txError();
  }
}

// Note: Since txTick() takes a significant amount of time, the
// fastest the metronome will currently go is 240 bpm.
void txMetronome(MorseToken) {
  uint8_t bpm(60);
  switch (symbolStackSize()) {
  case 3: 
    bpm = 100 * m2i(s(2)) + 10 * m2i(s(1)) + m2i(s(0));
    popN(3);
    break;
  case 2: 
    bpm = 10 * m2i(s(1)) + m2i(s(0));
    popN(2);
    break;
  case 1:
    bpm = m2i(s(0));
    popN(1);
    break;
  default: txError();
  }
  uint32_t timeout(60 * 1000000 / bpm);
  elapsedMicros sinceTick(0);
  while (!touchPoll(dahPin)) {
    if (timeout < sinceTick) {
      sinceTick = 0;
      txTick(MorseToken());
    }
  }
}

void sumStack(MorseToken) {
  int sum(0);
  for (size_t i(0); i < symbolStackSize(); ++i) {
    sum += m2i(s(i));
  }
  txUnsigned(sum);
}

void guitarTune(MorseToken) {
  // EADGBE
  playTone(noteHz[40] + 0.5, getDitMicros() * 6);
  playTone(noteHz[45] + 0.5, getDitMicros() * 6);
  playTone(noteHz[50] + 0.5, getDitMicros() * 6);
  playTone(noteHz[55] + 0.5, getDitMicros() * 6);
  playTone(noteHz[59] + 0.5, getDitMicros() * 6);
  playTone(noteHz[64] + 0.5, getDitMicros() * 6);
}

void setAlarm(MorseToken) {
  Serial.print("Setting alarm at ");
  Serial.print(m2a(s(3)));
  Serial.print(m2a(s(2)));
  Serial.print(':');
  Serial.print(m2a(s(1)));
  Serial.println(m2a(s(0)));
}

uint8_t uptime(0);

void updateUptime() {
  EEPROM.write(0, ++uptime);
}

void setUptime(uint8_t u) {
  uptime = u;
}

void txUptime(MorseToken) {
  Serial.print("uptime: ");
  Serial.println(uptime);
  txUnsigned(uptime);
}
