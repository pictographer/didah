// -*- mode: C++; -*-
///\file libmorse.h Morse code operations
#pragma once
#include "binconst.h"
#include <cstdio>
#include <cstring>
#include <cinttypes>
#include <climits>
#include "teensy3_morse_bsp.h"
#include "morse_token.h"

///\name Morse Code
//@{

/// A dit lasts one unit of time.
const int ditFactor = 1;

/// The dah duration is this factor times the dit duration.
const int dahFactor = 3;

/// The inter-letter gap is this factor times the dit duration.
const int letterGapFactor = 3;

/// The inter-word gap is this factor times the dit duration.
const int wordGapFactor = 7;

extern const char* morsePunctuation;
//@}

///\name Copying the user
//@{

/// Has it been at least 600 word gaps since the last activity?
bool isIdle();

/// Returns true if longer than 5 units have elapsed.
bool letterExpired();

///\brief Has it been long enough that the word under construction
/// should be considered complete?
bool wordExpired();

///\brief Accumulate a Morse token.
///
/// If the token is in progress, return an invalid token. If the token
/// is complete, return it. The caller is expected to call this
/// function repeatedly until a token is completed or until the caller
/// decides to stop for some other reason, e.g. timeout.
///
/// The two buttons function like an iambic keyer. Since a dit or dah
/// is echoed after it is keyed in, and during this time the
/// application is not listening or buffering, there's no need for
/// additional debouncing logic.
MorseToken getMorse();

///\brief On my hardware there is cross-coupling between the serial
/// port and the touch sensors. Without the code below, long messages
/// to the serial port trigger a touch input.
void flushTouch();

//@}

///\name Transmission
//@{

/// Transmit a literal Morse code value.
void txLiteral(MorseToken code);

/// Transmit a Morse code prosign indicating an error. 8 dits.
void txError();

/// Convert an ASCII character into Morse code and transmit it.
void txLetter(char a);

/// Convert an unsigned integer into a sequence of Morse code numerals
/// and transmit them.
void txUnsigned(uint16_t n);

/// Transmit the 24-hr time.
void txTimeHHMM(MorseToken code);

/// Transmit a string as Morse code. Prosigns are delimited with
/// carets instead of underscores, e.g.
///    "^SOS^ ^SOS^ ^SOS^ DE GBTT GBTT GBTT QUEEN ELIZABETH 2"
void txString(const char* s);

//@}

///\name Serial I/O
//@{

void keyboardMode(MorseToken);
void rxString(char* buffer, size_t n);

template<typename T>
void printLabelValueUnits(const char* label,
			  const T value,
			  const char* units = "")
{
  Serial.print(label);
  Serial.print(value);
  Serial.println(units);
}

//@}


