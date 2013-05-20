// -*- mode: C++; -*-
// Didah - A Morse code PDA for the Teensy 3.0 ARM Cortex-M4 development board
// Copyright © 2013 The Pictographer's Shop.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "WProgram.h"
#include <EEPROM.h>
#include "mac.h"
#include "libmorse.h"
#include "actions.h"
#include "prosign.h"

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(beepPin, OUTPUT);

  analogRead(ditPin);   // Allow callibration to complete
  analogReadRes(16);    // Full resolution
  ADC0_SC3 = 0;         // No averaging
  
  delay(500); // Give the finger a chance to get off the reset button.
  calibrateTouch(ditPin);
  calibrateTouch(dahPin);
  
  setUptime(EEPROM.read(0));//XXX not uptime--reprogram time

  initActions();
  for (char c('A'); c <= 'Z'; ++c) {
    assignAction(a2m(c), txEcho);
  }
  for (size_t i(0); i < 10; ++i) {
    assignAction(a2m(i + '0'), pushSymbolStack);
  }
  assignAction(a2m('A'), dispatchQCode);
  assignAction(a2m('B'), dispatchQCode);
  assignAction(a2m('C'), dispatchQCode);
  assignAction(a2m('D'), playNote);
  assignAction(a2m('F'), txInternalTemperature);
  assignAction(a2m('G'), guitarTune);
  assignAction(a2m('H'), askSum);
  assignAction(a2m('I'), gradeSum);
  assignAction(a2m('J'), askNextAlfa);
  assignAction(a2m('K'), dispatchQCode);
  assignAction(a2m('L'), txListSymbolStack);
  assignAction(a2m('M'), txMetronome);


  assignAction(a2m('P'), popSymbolStack);
  assignAction(a2m('Q'), dispatchQCode);
  assignAction(a2m('R'), dispatchQCode);
  assignAction(a2m('S'), dispatchQCode);
  assignAction(a2m('T'), dispatchQCode);
  assignAction(a2m('U'), txUptime);
  assignAction(a2m('V'), txSymbolStackTop);
  assignAction(a2m('W'), playScale);
  assignAction(a2m('X'), dispatchQCode);
  assignAction(a2m('Y'), getPressure);
  assignAction(a2m('Z'), playSoundEffect);

  //  assignAction(a2m('K'), doMatchScore);
  //  assignAction(a2m('N'), tx0to9);
  //XXX needs to be longer! Dangerous.
  //  assignAction(a2m('N'), setVolume);
  //  assignAction(a2m('X'), exchangeStack);

  // XXX Need a more appropriate command that ^CT^.
  assignAction(MorseToken(MORSE_CT), keyboardMode);
  assignAction(MorseToken(MORSE_INTER), dispatchQCode);
  assignAction(a2m('+'), sumStack);

  Serial.begin(38400);		// Does not block.
  
  txGreeting(MorseToken());
  Serial.println(identifyProcessor());
  txString(identifyProcessor());
}

void loop() {
  // Gather Morse code symbols from touch input and invoke an
  // evaluation function on them.
  MorseToken token(getMorse());
  if (token.valid()) {
    Serial.println(m2a(token));

    doAction(token);
    
  } else if (isIdle() && Teensy3Clock.get() % (60 * 15) == 0) {
    // Announce the time every 15 minutes.

    // Make a quiet static sound to avoid startling the user before
    // beeping the time.
    phonographHiss(8 * ditMicrosDefault);
    txTimeHHMM(MorseToken());
    updateUptime();
  }
}
