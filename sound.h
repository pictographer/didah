/// PWM sound effects

///\file sound.h
// -*- mode: C++; -*-
#pragma once

#include <cctype>
#include <cinttypes>

/// Play a hiss like an old phonograph record.
void phonographHiss(uint32_t duration);

/// Play a PWM tone at a given frequency and duration.
void playTone(size_t frequency, size_t duration);

/// Play a whooping sound like a toy ray gun.
void rayGun(uint32_t duration);

/// Shuffling cash like a bill counter
void shufflingCash(uint32_t duration);
