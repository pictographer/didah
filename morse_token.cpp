///\file morse_token.cpp Implementation of Morse code operations
#include "morse_token.h"
#include "actions.h" // for txError

#define MORSE_UNDEFINED_CHAR '^'

/// Morse code for letters encoded as an implicit binary tree.
/// Letters are written in breadth first order. Left child
/// corresponds to dit; right child corresponds to dah.
/// Caret is used for undefined (or incomplete) codes.
/// The root of the tree acts as a sentinel to simplify the process of
/// converting the loop index into our left-justified code scheme.
///
/// Here are some examples to show how it works:
///
///    Index     | Letter | Morse Code   | Notes
///    ----------|--------|--------------|------
///    \c 000000 | N/A    | N/A          | Invalid code
///    \c 000001 | space  | N/A          | Sentinel followed by zero bits (represents a pause)
///    \c 000010 | \c E   | `.`          | Sentinel followed by a dit
///    \c 000011 | \c T   | `_`          | Sentinel followed by a dah
///    \c 000100 | \c I   | `. .`        | Sentinel followed by two dits
///    \c 000101 | \c A   | `. _`        | And so on...
///    \c 000110 | \c N   | `_ .`        | &nbsp;
///    \c 000111 | \c M   | `_ _`        | &nbsp;
///    \c 001000 | \c S   | `. . .`      | &nbsp;
///    \c 001001 | \c U   | `. . _`      | &nbsp;
///    \c 001010 | \c R   | `. _ .`      | &nbsp;
///    \c 001011 | \c W   | `. _ _`      | &nbsp;
///    \c 001100 | \c D   | `_ . .`      | &nbsp;
///    \c 001101 | \c K   | `_ . _`      | &nbsp;
///    \c 001110 | \c G   | `_ _ .`      | &nbsp;
///    \c 001111 | \c O   | `_ _ _`      | &nbsp;
const char morseTree[] =
  "^^"
  "ET"                                                    // 1 symbol
  "IANM"                                                  // 2 symbols
  "SURWDKGO"                                              // 3 symbols
  "HVF^L\nPJBXCYZQ!^"                                     // 4 symbols
  "54^3^^^2&/+^^^^16=/^^^(^7^^^8^90"                      // 5 symbols
  "^^^^$^^^^^^^?_^^^^\"^^.^^^^@^^^'^^-^^^^^^^^;!^)^^^^^,^^^^:^^^^^^";

const char* morsePunctuation = "!\"$&'()+,-./:;=?@_";

/// Convert a Morse code into ASCII.
char m2a(MorseToken code) { 
  char result('x');
  if (!code.valid()) {
    ; // Return 'x' for error.
  } else if (code.pause()) {
    result = ' ';
  } else {
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
      // Translate the subset of the prosigns/abbreviations that can
      // be represented by a single ASCII character, but are not
      // present in morseTree above. These tokens have 7 symbols. Due
      // to a limitation of the getMorse() function, we never get
      // here. Dollary sign is recognized as a six symbol prefix of
      // its seven symbol code.
      //
      // Note: MORSE_ERROR does not fit in the 8-bit representation of
      // MorseToken.
      switch (raw) {
      case MORSE_DOLLAR: result = '$'; break;
      default:
	; // Nothing more to do since result is initialized to 'x'.
      }
    }
  }
  return result;
}

/// Convert a single ASCII character into a Morse code token.
MorseToken a2m(char a) {
   uint8_t letterCode(0);
   ptrdiff_t key(strchr(morseTree, a) - morseTree);
   if (0 < key && morseTree[key] != MORSE_UNDEFINED_CHAR) {
      // Add the sentinel to the right.
      letterCode = key << 1 | 1;
      while (!(letterCode & MORSE_MSB)) {
         letterCode <<= 1;
      }
      letterCode <<= 1;
   } else {
      // The codes below would require more rows in morseTree and
      // the rows would be mostly empty, so they are handled here.
      switch (a) {
      case '\n': letterCode = MORSE_AA; break;
      case '&':  letterCode = MORSE_AMPER; break;
      case ')':  letterCode = MORSE_CLOSEP; break;
      case '$':  letterCode = MORSE_DOLLAR; break;
      default:
         ; // Nothing more to do since letterCode is initialized to 0
         // (invalid).
      }
   }
   return MorseToken(letterCode);
}

/// The given Morse code token should be a digit. Return the
/// corresponding number. If the input is not a digit, transmits the
/// error prosign and returns an undefined value.
uint8_t m2i(MorseToken m) {
  uint8_t result(m.toChar() - '0');
  if (9 < result) {
    txError();
  }
  return result;
}

/**
Given an ASCII string representing a prosign, return a 
code composed by concatenating the morse code for each 
character in the prosign without inter-symbol gaps.

For example "AB" yields

String | Morse Code                | Binary
-------|---------------------------|-------
\c A   | \c . _                    | \c 01100000
\c B   | \c &nbsp;&nbsp;_ . . .    | \c &nbsp;&nbsp;10001000
\c AB  | \c . _ _ . . .            | \c 01100010

If the translation is longer than will fit in MorseToken, the return
value will be the longest suffix of the input that does fit. All the
prosigns I'm aware of fit without truncation if MorseToken is 16 bits
or more. Most fit in 7 bits with the notable exception of 
\c MORSE_ERROR, which requires 9 bits (8 symbols + 1 sentinel).
Unfortunately, there are many ways of denoting 8 dits, so the special
case isn't so easy to code, e.g. \c ^HH^, \c ^5S^, \c ^S5^, \c ^IIII^,
\c ^IIH^, \c ^EEEEEEEE^, etc.
*/
MorseToken a2p(const char* prosign) {
   uint8_t letterCode(MorseToken(prosign[0], MorseToken::Char));
   uint8_t t(letterCode);
   size_t length(0);
   while (t) { ++length; t <<= 1; }
   const char* p(prosign + 1);
   while (*p) {
      char a(*p);
      uint8_t m(MorseToken(a, MorseToken::Char));
      // Erase the sentinel (always 1).
      letterCode ^= 1 << (wordBits - length);
      // Shift the new symbol and or it in after the previous.
      letterCode |= m >> (length - 1);
      // Update the length.
      while (m) { ++length; m <<= 1; }
      ++p;
   }
   return MorseToken(letterCode);
}

