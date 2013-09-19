// -*- mode: C++; -*-
/**\file teensy3_morse_bsp.h
\brief Teensy 3.0 board support package
 */
#pragma once
#include "Arduino.h"

/// Return a pointer to a static string that identifies the processor chip.
char* identifyProcessor();

/// Establish a baseline for a touch sensor when no touch is present.
void calibrateTouch(uint8_t pin);

/// Return the capacitance threshold in internal units of approximately
/// pF/20 per count.
uint16_t getPinThreshold(uint8_t pin);

/// Use calibration data to determine whether or not a touch is present
/// on a given sensor.
bool touchPoll(uint8_t pin);

/// 50% duty cycle for analogWrite
const size_t duty50 = 255/2;

size_t getDuty();

void setDuty(uint8_t);

bool isEarphonePresent();

/// Generate a tone using PWM.
void beep(uint32_t frequency, uint32_t durationMicros);

/// Teensy 3.0 LED
const int ledPin = 13;          // Not used in the appliance.

/// Pin attached to a small speaker in the PDA hardware.
const int beepPin = 10;

const int earphoneRightDetectPin = 3;
const int earphoneRightPin = 4;

/// Note: earphoneLeftPin isn't actually wired up in the appliance.

const int piezoTxP = 22;
const int piezoTxN = 20;

/// Sends audio to the radio.
const int txPin = 23;

/// Provides "push to talk" open collector enabling the radio.
const int txEnablePin = 14;

const int ledRedP = 27;
const int ledRedN = 30;
const int ledGreenP = 29;
const int ledGreenN = 28;

#if defined(PDA)

/// A touchRead() pin. Auto-repeats a dit.
const int ditPin = 0;

/// A touchRead() pin. Auto-repeats a dah.
const int dahPin = 1;

#else

/// A touchRead() pin. Auto-repeats a dit.
const int ditPin = 19;

/// A touchRead() pin. Auto-repeats a dah.
const int dahPin = 17;

#endif

/// Voltage measurement 0 - 3.3V
const int analogInput = 21;

const int temperatureSensor = 38;

bool getSenseMode();

void setSenseMode(bool mode);

/// Transmit the internal temperature in integer Celcius.
/// From https://community.freescale.com/thread/304728
float getInternalTemperatureC();

///\brief Return the most recent ADC measurement of the input voltage.
/// Perform analog to digital conversion on the analogInput pin,
/// callibrated to the protection diode and voltage divider on the
/// interface/power board.
int32_t readMillivolts();

/// Increment the voltage sampling and filtering.
void sampleVoltage();

/// Report register settings related to the external clock crystal.
void describeRTC();

/// Sense whether or not a realtime clock crystal is present.
bool hasRTC();

///\brief Initialize the input and output ports used by appliance or
/// PDA hardware.
void initPorts();

///\brief Set the PWM frequency for the audio output pins.
void pwmFrequency(int Hz);

///\brief Generate a tone on the audio output pins.
void pwmWrite(int duty);

///\brief Perform digital writes to all the output pins.
void dWrite(int value);

///\brief Turn the red LED on (1) or off (0).
void redLED(int value);

///\brief Turn the green LED on (1) or off (0).
void greenLED(int value);

///\brief Change the state of the green LED.
void toggleGreenLED();

///\brief Change the state of the red LED.
void toggleRedLED();

///\brief Get the raw analog input.
uint32_t getRawVoltage();
