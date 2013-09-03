// -*- mode: C++; -*-
///\file pda.h
///\brief Personal Digital Assistant functions
#pragma once
#include "libmorse.h"

void txTimeHHMM(MorseToken code);

/// Play a series of tones randomly bounded by the reading of the dit
/// pin.
void pressureTone(uint16_t duration);

void playScale(MorseToken code);
void playSoundEffect(MorseToken cord);
void playNote(MorseToken code);
uint8_t durationDits(char a);
void txAlphabetSong(MorseToken code);
void txAlfaBravoCharlie(MorseToken code);
void txAlfaNNN(MorseToken code);
void askSum(MorseToken);
void gradeSum(MorseToken);
void askNext(MorseToken);
void askNextAlfa(MorseToken);
void gradeRestore(MorseToken answer);
void gradeNext(MorseToken);
void txMetronome(MorseToken);
void sumStack(MorseToken);
void guitarTune(MorseToken);
void setAlarm(MorseToken);
