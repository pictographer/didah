///\file libmorse.cpp Implementation of Morse code operations
#include <cstring>
#include <limits>
#include "Arduino.h"
#include "libmorse.h"
#include "settings.h"
#include "stack.h"
#include "actions.h"
#include "prosign.h"

#include "Time.h"

//---- Copying the user ----

/// The end of the user's input is determined by a timeout
/// equal to two times the normal word gap.
elapsedMicros sinceTouch;

///\var Morse code reversed with sentinel.
uint16_t leftCode = 1;

///\var Number of dits and dahs in the current symbol.
uint8_t touchCount = 0;

bool isIdle() {
  return leftCode == 1 &&
    600U * wordGapFactor * getDitMicros() < sinceTouch;
}

bool letterExpired() {
  const unsigned int ImSlow(5);
  return ImSlow * getDitMicros() < sinceTouch;
}

bool wordExpired() {
  const unsigned int ImSlow(20);
  return ImSlow * getDitMicros() < sinceTouch;
}

MorseToken getMorse() {
  bool ePin(touchPoll(ditPin));
  bool tPin(touchPoll(dahPin));
  
  if (ePin || tPin) {
    sinceTouch = 0;
    ++touchCount;
  }

  uint32_t saveTxHz(getTxHz());

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
  setTxHz(saveTxHz);

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

void flushTouch() {
  sinceTouch = 0;
  touchCount = 0;
  leftCode = 1;
  touchPoll(ditPin);
  touchPoll(dahPin);
  delayMicroseconds(100000);
}

//---- Transmission ----

/// Transmit a Morse code prosign indicating an error.
void txError() {
  Serial.print("Error [........] ");
  txString("EEEEEEEE");
}

/// Return a string of dots and dashes corresponding to code.
const char* m2didah(MorseToken code) {
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

/// Transmit a Morse code symbol. The parameter is encoded as a
/// left-justified bit sequence where 0 represents dit and 1
/// represents dah. Since Morse code symbols vary in length, the code
/// is terminated with a 1 after the right- most token.
void txLiteral(MorseToken code) {
   if (code.valid()) {		// Guard against an infinite loop.
      uint8_t raw(code);
      while (raw != MORSE_MSB) {
         beep(getTxHz(),
              getDitMicros() * (raw & MORSE_MSB ? dahFactor : ditFactor));
         Serial.print(raw & MORSE_MSB ? "_" : ".");
         delayMicroseconds(getDitMicros());
         raw <<= 1;
      }
      Serial.print(' ');
      delayMicroseconds((letterGapFactor - ditFactor) * getDitMicros());
   } else {
      txError();
   }
}

/// Transmit a Morse code symbol consisting of one letter or
/// number. The ASCII-to-Morse translation table is represented as an
/// implicit binary tree. See the header file for details.
///
/// If x is not in the set [A-Z0-9], transmit an 'error' prosign.
void txLetter(char a) {
   MorseToken letterCode(a, MorseToken::Char);
   if (letterCode != MORSE_ERROR) {
      txLiteral(letterCode);
   } else {
      txError();
      Serial.println(a);
   }
}

/// Transmit an unsigned integer in Morse code.
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

/// Transmit a Morse code string. Prosigns are delimited by carets
/// (^), e.g. "^QTR^ 2112". Caret is used instead of underscore so
/// that underscores can be transmitted without quoting them or
/// encoding them somehow.
///
/// Several format placeholders are supported.
///
///\li    %C - Transmit the temperature in degrees Celsius
///\li    %F - Transmit the temperature in degrees Fahrenheit
///\li    %T - Transmit the 24-hour time as four-digits
///\li    %U - Transmit the uptime in quarter hours
///\li    %V - Transmit the millivolts measured by pin analogInput
///\li    %% - Transmit a literal percent sign (not that anyone will
///            understand this obscure Morse code)
///
/// Abort if the dit key is touched.
void txString(const char* s) {
   bool gotPercentSign(false);
   if (*s) {
      for (char* p(const_cast<char*>(s));
           *p && !touchPoll(ditPin);
           ++p)
         {
            if (*p == ' ') {
               Serial.print('/');
               delayMicroseconds(wordGapFactor * getDitMicros());
            } else if (*p == '^') {
               // Accumulate the following prosign and transmit it.
               char pbuffer[5] = { '1', '2', '3', '4', '\0' };
               char* q(pbuffer);
               while (*q && *++p != '^') *q++ = *p;
               *q = 0;
               txLiteral(MorseToken(pbuffer, MorseToken::Prosign));
            } else if (gotPercentSign) {
               switch (*p) {
               case '%': txLetter(*p); break;
               case 'C':
               {
                  uint16_t t(getInternalTemperatureC());
                  txUnsigned(t);
               }
               break;
               case 'F':
                  txInternalTemperatureF(MorseToken());
                  break;
               case 'T': txTimeHHMM(MorseToken()); break;
               case 'U': txUptime(MorseToken()); break;
               case 'V': txUnsigned((uint16_t) readMillivolts()); break;
               default:
                  txLetter('%');
                  txLetter(*p);
               }
               gotPercentSign = false;
            } else if (*p == '%') {
               gotPercentSign = true;
            } else {
               txLetter(*p);
            }
         }
      delayMicroseconds((wordGapFactor - letterGapFactor) * getDitMicros());
      Serial.println();
      Serial.println(s);
      flushTouch();
   }
}

//---- Pattern Matching ----

uint8_t matchScore(const char* pattern) {
   uint8_t score(255);
   const char* patEnd(pattern);
   while (*patEnd) ++patEnd;
   const char* patSym(patEnd);
   if (patSym == pattern) {
      score = 0;
      // Null pattern never matches.
      return score;
   }
   --patSym;
   size_t sp(0);
   bool sawDigit(false);
   do {
      char si(s(sp).toChar());
      switch (*patSym) {
      case 'a':
         // Match any (simple greedy match).
         // Must occur at the beginning of the pattern.
         return score;
         break;
      case 'c':
         // Match a character.
         score -= score && !isalnum(si);
         break;
      case 'd':
         // Match a digit.
         score -= score && (uint8_t(9U) < uint8_t(si - '0'));
         break;
      case 'n':
         // Match one or more digits.
         // Note: only the stack size limits the number of digits.
         if (uint8_t(si - '0') < 10U) {
            sawDigit = true;
            // Don't advance to the next character of pattern.
            ++patSym;
         } else {
            if (!sawDigit) {
               score -= !!score;
            } else {
               // Finished matching n. Advance the pattern, but reuse
               // the current stack element.
               --sp;
            }
         }
         break;
      case '^': {
         // Match a prosign.
         // Note: This code is just enough different than txString to
         // be annoying.
         char pbuffer[5] = { '1', '2', '3', '4', '\0' };
         char* pb(pbuffer);
         --patSym;
         while (*patSym && (*patSym != '^')) {
            *pb++ = *patSym--;
         }
         --patSym;
         *pb-- = 0;
         // Reverse pbuffer since we're matching backwards, but the
         // prosign is a single multi-character entity.
         char* fb(pbuffer);
         while (fb < pb) {
            char temp(*fb);
            *fb++ = *pb;
            *pb-- = temp;
         }
         score -= score &&
            uint8_t(MorseToken(pbuffer, MorseToken::Prosign)) != s(sp);
         break;
      }
      default:
         // Match the literal value of c.
         score -= score && si != uint8_t(*patSym);
      };
   } while (pattern < patSym-- && ++sp != symbolStackSize());

   if (pattern != patSym && sp == symbolStackSize()) {
      // If the pattern wasn't used completely deduct a point for
      // ever unused pattern symbol.
      score -= patSym - pattern;
   }

   return score;
}

//!\todo seems application-specific
void keyboardMode(MorseToken) {
   txString("KB");
   bool didSpace(true);
   elapsedMicros sinceKeyboard(0);
   MorseToken al("AL", MorseToken::Prosign);
   MorseToken ar(MORSE_AR);
   MorseToken bt(MORSE_BT);
   for (MorseToken m(getMorse()); m != ar; m = getMorse()) {
      if (m.valid()) {
         if (m == bt) {
            Keyboard.print("\n");
            Serial.print("\n");
            didSpace = true;
         } else if (m == al) {
            Keyboard.print("\n\n");
            Serial.print("\n\n");
            didSpace = true;
         } else {
            const char c(tolower(m.toChar()));
            Keyboard.print(c);
            Serial.print(c);
            didSpace = false;
         }
         sinceKeyboard = 0;
      } else if (m.pause() &&
                 (20U * getDitMicros() < sinceKeyboard) && !didSpace)
      {
         txTock(MorseToken());
         Keyboard.print(' ');
         Serial.print(' ');
         didSpace = true;
      }
   }
   txString("?");
}

/// Receive keying into buffer until \em AR prosign is received,
///timeout, or buffer full.
///\note n should be positive. If n == 0, a null will be written to
///buffer[0] and the function will return immediately.
void rxString(char* buffer, size_t n) {
   char* p(buffer);
   const char* e(buffer + n - 1);
   bool didSpace(true);
   elapsedMicros sinceKeyboard(0);
   char c0(' ');
   char c1(' ');
   MorseToken ar(MORSE_AR);
   for (MorseToken m(getMorse()); p < e &&  m != ar; m = getMorse()) {
      if (m.valid()) {
         const char c(m.toChar());
         if (c0 == ' ' &&
             c1 == '?' &&
             (c == 'V' || c == 'F' || c == 'C' || c == 'T'))
         {
            // Convert " ?V" to " %V" so that voltage is substituted
            // on transmission. Likewise for ?C, ?F, and ?T.
            *(p - 1) = '%';
            Serial.print('%');
         }
         *p++ = c;
         c0 = c1;
         c1 = c;
         Serial.print(c);
         didSpace = false;
         sinceKeyboard = 0;
      } else if (m.pause() &&
                 (20U * getDitMicros() < sinceKeyboard) && !didSpace)
         {
            txTock(MorseToken());
            c0 = c1;
            c1 = ' ';
            *p++ = ' ';
            Serial.print(' ');
            didSpace = true;
         } else if (m.pause() && (200U * getDitMicros() < sinceKeyboard)) {
            Serial.println("Timeout");
            break;
         }
   }
   *p = 0;
   if (p == e) {
      txString("FULL");
   }
   txString("?");
}
