// -*- mode: C++; -*-
#pragma once
#include "binconst.h"
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <climits>
#include "teensy3_morse_bsp.h"

//---- Morse Code ----

// A dit lasts one unit of time.
const int ditFactor(1);

// The dah duration is this factor times the dit duration.
const int dahFactor(3);

// The inter-letter gap is this factor times the dit duration.
const int letterGapFactor(3);

// The inter-word gap is this factor times the dit duration.
const int wordGapFactor(7);

// Frequency of 440 Hz
const int A440(440);

// Duration of a dit in microseconds
const int ditMicrosDefault(40000);

void setDitMicros(int ms);
int getDitMicros();

uint32_t getTxHz();
void setTxHz(uint32_t Hz);

//---- A Morse code token ----

// Each Morse code symbol is encoded as a bit vector, processed from
// MSB to LSB with 0 corresponding to dit and 1 corresponding to
// dah. Since Morse code symbols vary in length, a 1 bit follows the
// last dit/dah and remaining low order bits are all zero. This makes
// it convenient to serialize a code word by left shifting one bit at
// a time stopping when the remainder has only the MSB set.

class MorseToken {
 public:
  explicit MorseToken() : m(0) {}
  explicit MorseToken(uint8_t m) : m(m) {}
  operator uint8_t() const { return m; }
  bool valid() { return 1 < m; }
 private:
  uint8_t m;
};

//---- Copying the user ----

// Has it been at least 600 word gaps since the last activity?
bool isIdle();

bool letterExpired();

// Has it been long enough that the word under construction
// should be considered complete?
bool wordExpired();

// Accumulate a Morse token. If the token is in progress, return an
// invalid token. If the token is complete, return it. The caller is
// expected to call this function repeatedly until a token is
// completed or until the caller decides to stop for some other
// reason, e.g. timeout.
MorseToken getMorse();

//---- Actions ----

typedef void (*ActionFn)(MorseToken);

// Initialize the table of function pointers to noops.
void initActions();

// Associate a Morse code symbol with an action.
void assignAction(MorseToken code, ActionFn action);

// Noop
void doNothing(MorseToken code);

// Invoke the action associated with the given Morse code symbol.
void doAction(MorseToken code);

void saveActions();
void restoreActions();
void push1RestoreAll(MorseToken code);

//---- Conversion ----

const size_t wordBits(CHAR_BIT * sizeof MorseToken(0));

// Convert Morse code (internal notation: reversed right justified
// with sentinel on the left) to ASCII.
char m2a(MorseToken code);

// Convert a single ASCII letter, number, or a few operators to 
// Morse code.
MorseToken a2m(char a);

// Convert Morse 0-9 to the corresponding digit.
uint8_t m2i(MorseToken m);

// Convert an ASCII string into a Morse prosign. Note: the result
// must fit in 16-bits including the sentinel.
MorseToken a2p(char* prosign);

// Wacky stand-alone function for blinking out a single digit
// from 0-9 in Morse code. What's wacky?
//    * The function uses a single eight bit constant as the Morse
//      code table for the 10 values, 
//    * The function depends on sign extension, and
//    * The function fits in 80 bytes of ARM code.
//
// I imagine this function as the gateway drug of Morse code
// debugging.
void blinkMorseDigit(uint8_t digit);

// Two attempts at making blinkMorseDigit() shorter, weighing in at
// 96 and 104 bytes respectively.
void bmd096(uint8_t digit);
void bmd104(uint8_t digit);

// Transmit a Morse code prosign indicating an error. 8 dits.
void txError();

//---- Command interpretation ----

// Just your basic noop.
void doNothing(MorseToken);

// Load the table of actions with noops.
void initActions();

// Our internal representation of Morse code is left justified, that
// is to say, the high order bit is the first dit/dah. Most of the
// time, the bottom 8 bits will be zero, so create a hash by grabbing
// the top 8 bits.
uint8_t codeHash(MorseToken code);

// Hash a Morse code symbol to a table location and store action there.
void assignAction(MorseToken code, ActionFn action);

// Invoke an action from the actions table.
void doAction(MorseToken code);

// Save all the action pointers in preparation for overwriting some or
// all of them with action pointers for a mode.
void saveActions();

// If there are actions saved, restore them. Otherwise, transmit
// an error.
void restoreActions();

// Push the most recent symbol onto the stack, and then restore
// all the actions to their previous values.
void push1RestoreAll(MorseToken code);

//XXX This mixes library stuff with actions stuff.
//XXX Belongs in actions.cpp?
void gradeRestore(MorseToken answer);

// The given Morse code token should be a digit. Return the
// corresponding number. If the input is not a digit, transmits the
// error prosign and returns an undefined value.
uint8_t m2i(MorseToken m);

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
// If the translation is longer than will fit in MorseToken, the return
// value will be the longest suffix of the input that does fit. All
// the prosigns I'm aware of fit without truncation if MorseToken is 16
// bits or more.
MorseToken a2p(char* prosign);

//---- Conversion ----

// Convert a Morse code into ASCII.
char m2a(MorseToken code);

// Convert a single ASCII character into a Morse code token.
MorseToken a2m(char a);

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
void blinkMorseDigit(uint8_t digit);

// Blink a single Morse code digit.
// Implemented with a table lookup.
// 96 bytes
void bmd096(uint8_t digit);

// Blink a single Morse code digit.
// Implemented implemented with a table lookup and the observation
// that Morse(x) == ~Morse(x + 5) when x < 5.
// 104 bytes
void bmd104(uint8_t digit);

// Transmit a literal Morse code value.
void txLiteral(MorseToken code);

// Convert an ASCII character into Morse code and transmit it.
void txLetter(char a);

// Convert an unsigned integer into a sequence of Morse code numerals
// and transmit them.
void txUnsigned(uint16_t n);

// Transmit a string as Morse code. Prosigns are delimited with
// underscores, e.g.
//    "^SOS^ ^SOS^ ^SOS^ DE GBTT GBTT GBTT QUEEN ELIZABETH 2"
void txString(const char* s);

//---- Pattern Matching ----

uint8_t matchScore(const char* pattern);

//---- Keyboard ----

void keyboardMode(MorseToken);

void setVolume(MorseToken);

