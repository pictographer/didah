#pragma once
#include <cinttypes>
#include <climits>
#include <cstddef>
#include "prosign.h"

//#include <Arduino.h>  // for type definitions

/**\file morse_token.h
\brief Defines the MorseToken class and various helpers.

Each Morse code symbol is encoded as a bit vector, processed from
MSB to LSB with 0 corresponding to dit and 1 corresponding to
dah. Since Morse code symbols vary in length, a 1 bit follows the
last dit/dah and remaining low order bits are all zero. This makes
it convenient to serialize a code word by left shifting one bit at
a time stopping when the remainder has only the MSB set.
*/
class MorseToken;

/// Mask for the high order bit of a MorseToken field.
#define MORSE_MSB (1 << (CHAR_BIT * sizeof MorseToken(0) - 1))

/// Convert Morse code (internal notation: reversed right justified
/// with sentinel on the left) to ASCII.
char m2a(MorseToken code);

/// Convert a single ASCII letter, number, or a few operators to 
/// Morse code.
MorseToken a2m(char a);

/// Convert Morse 0-9 to the corresponding digit.
uint8_t m2i(MorseToken m);

MorseToken a2p(const char* prosign);

///\brief Represents an individual Morse code token, such as a letter,
/// pause, or prosign.
///
class MorseToken {
public:
   typedef void (* SymbolFn)(bool);

public:

   /// Construct an invalid default morse token.
   explicit MorseToken() : m(0) {}

   /// Convert a binary value into a MorseToken.
   explicit MorseToken(uint8_t m) : m(m) {}

   /// Constructor disambiguation tag.
   enum char_t { Char };
   explicit MorseToken(char a, char_t) : m(a2m(a)) {}

   /// Convert a MorseToken to a binary value.
   operator uint8_t() const { return m; }

   /// Constructor disambiguation tag.
   enum prosign_t { Prosign };
   explicit MorseToken(const char* prosign, prosign_t) : m(a2p(prosign)) {}

   /// Treat code == 0 as the error token.
   bool valid() { return 1 < m; }

   /// Treat code == 1 as a space.
   bool pause() { return m == 1; }

   /// Convert to a single ASCII character.
   char toChar() { return ::m2a(*this); }

   /// Convert Morse 0-9 to the corresponding digit.
   uint8_t m2i() { return ::m2i(*this); }

   /// Invoke fn() on each bit of this Morse token, in tranmission order.
   void foreachSymbol(MorseToken code, SymbolFn fn) {
      if (valid()) {
         for (uint8_t raw(m); raw != MORSE_MSB; raw <<= 1) {
            fn(raw & MORSE_MSB);
         }
      }
   }
private:
   /// Internal representation of a Morse code token.
   uint8_t m;
};

//---- Conversion ----

const size_t wordBits(CHAR_BIT * sizeof MorseToken());

