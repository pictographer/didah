///\file morse_digit.cpp
///\brief Implementation of decimal digit to Morse code conversion
#include "teensy3_morse_bsp.h"
#include "settings.h"
#include "actions.h"
#include "morse_digit.h"

void blinkMorseDigit(uint8_t digit) {
  const signed char b111110(0x3E);
  const signed char dend(digit - 5);
  for (signed char i(digit); i != dend; --i) {
    beep(440,
	 getDitMicros() * (b111110 >> i & 1 ? ditFactor : dahFactor));
    delayMicroseconds(getDitMicros() *
		      (i - 1 == dend ? letterGapFactor : wordGapFactor));
  }
}

/// Blink a single Morse code digit.
/// Implemented with a table lookup.
/// 96 bytes
void bmd096(uint8_t digit) {
  uint8_t t[] = {
    B8(111111), B8(111110), B8(111100), B8(111000), B8(110000),
    B8(100000), B8(100001), B8(100011), B8(100111), B8(101111)
  };
  uint8_t m(t[digit]);
  while (m != 1) {
    beep(440, getDitMicros() * (m & 1 ? dahFactor : ditFactor));
    delayMicroseconds(getDitMicros() *
                      (m != 1 ? letterGapFactor : wordGapFactor));
    m >>= 1;
  }
}

/// Blink a single Morse code digit.
/// Implemented implemented with a table lookup and the observation
/// that Morse(x) == ~Morse(x + 5) when x < 5.
/// 104 bytes
void bmd104(uint8_t digit) {
  uint8_t t[] = { B8(11111), B8(11110), B8(11100), B8(11000), B8(10000) };
  uint8_t m(digit < 6 ? t[digit] : t[digit - 5] ^ B8(11111));
  m |= 1 << 5;
  while (m != 1) {
    beep(440, getDitMicros() * (m & 1 ? dahFactor : ditFactor));
    delayMicroseconds(getDitMicros() *
		      (m != 1 ? letterGapFactor : wordGapFactor));
    m >>= 1;
  }
}
