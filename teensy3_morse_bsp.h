// -*- mode: C++; -*-
#pragma once
#include "WProgram.h"

// Return a pointer to a static string that identifies the processor chip.
char* identifyProcessor();

// Establish a baseline for a touch sensor when no touch is present.
void calibrateTouch(uint8_t pin);

// Use calibration data to determine whether or not a touch is present
// on a given sensor.
bool touchPoll(uint8_t pin);

// 50% duty cycle for analogWrite
const size_t duty50(255/2);

size_t getDuty();

void setDuty(uint8_t);

// Generate a tone using PWM.
void beep(uint32_t frequency, uint32_t durationMicros);

// Teensy 3.0 LED
const int ledPin(13);

// Pin attached to a small speaker.
const int beepPin(10);

// A touchRead() pin. Auto-repeats a dit.
const int ditPin(0);

// A touchRead() pin. Auto-repeats a dah.
const int dahPin(1);

