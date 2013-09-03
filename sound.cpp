///\file sound.cpp
///\brief Implementation of PWM sound effects
#include "libmorse.h"
#include "sound.h"

/// Play a PWM tone at a given frequency and duration.
void playTone(size_t frequency, size_t duration) {
  pwmFrequency(frequency);
  pwmWrite(getDuty());
  delayMicroseconds(duration);
  pwmWrite(0);
  delayMicroseconds(duration);
}

/// Play a hiss like an old phonograph record.
void phonographHiss(uint32_t duration) {
  elapsedMicros t(0);
  while (t < duration) {
    dWrite(HIGH);
    delayMicroseconds((1 + random(100)) * 1136);
    dWrite(LOW);
    delayMicroseconds(100);
  }
}

/// Play a whooping sound like a toy ray gun.
void rayGun(uint32_t duration) {
  elapsedMicros t(0);
  while (t < duration) {
    for (int i(1); i < 25; ++i) {
      dWrite(HIGH);
      delayMicroseconds(i * 1136/12);
      dWrite(LOW);
      delayMicroseconds(i * 1136/12);
    }
  }
}

/// Shuffling cash like a bill counter
void shufflingCash(uint32_t duration) {
  elapsedMicros t(0);
  while (t < duration) {
    for (int i(1); i < 25; ++i) {
      dWrite(HIGH);
      delayMicroseconds((1 + random(i)) * 1136/12);
      dWrite(LOW);
      delayMicroseconds((1 + random(i)) * 1136/12);
    }
  }
}

