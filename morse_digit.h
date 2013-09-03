#pragma once
// -*- mode: C++; -*-
/** \file morse_digit.h 
\brief Three implementations of a function that converts a decimal
digit to Morse code.

These functions use a single 8-bit constant or small table of
constants to translate a number from 0 to 9 inclusive to the
corresponding Morse code.

\section license License Agreement

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

Copyright © 2013 The Pictographer's Shop.

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/** 
Transmit a digit in Morse code for memory-limited applications.
This wacky stand-alone function for blinking out a single digit
from 0-9 in Morse code.
   - The function uses a single eight bit constant \c 0b111110 with
     arithmetic shifts as the Morse code table for the 10 values, 
   - The function depends on sign extension, and
   - The function fits in 80 bytes of ARM code.

\param digit is the actual digit 0-9, not ASCII.

Morse code for the digits:
Digit | Morse Code | Binary
-----:|:----------:|-------
    0 | `-----`    |\c 11111
    1 | `.----`    |\c 01111
    2 | `..---`    |\c 00111
    3 | `...--`    |\c 00011
    4 | `....-`    |\c 00001
    5 | `.....`    |\c 00000
    6 | `-....`    |\c 10000
    7 | `--...`    |\c 11000
    8 | `---..`    |\c 11100
    9 | `----.`    |\c 11110

This is a standalone function. It does not use the left-justified
encoding scheme described in the header file.

This routine takes 80 bytes.

I imagine this function as the gateway drug of Morse code
debugging.
*/void blinkMorseDigit(uint8_t digit);

// Two attempts at making blinkMorseDigit() shorter, weighing in at
// 96 and 104 bytes respectively.

/// Blink a single Morse code digit.
/// Implemented with a table lookup.
/// 96 bytes
void bmd096(uint8_t digit);

/// Blink a single Morse code digit.
/// Implemented implemented with a table lookup and the observation
/// that Morse(x) == ~Morse(x + 5) when x < 5.
/// 104 bytes
void bmd104(uint8_t digit);

