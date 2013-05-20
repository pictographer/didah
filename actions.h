// -*- mode: C++; -*-
#pragma once
#include "WProgram.h"
#include "libmorse.h"

extern char timeBuf[6];
extern char mode;

void txTimeHHMM(MorseToken code);
void txInternalTemperature(MorseToken code);
void txMacAddress(MorseToken code);
void phonographHiss(int duration);
void playSoundEffect(MorseToken cord);
void playNote(MorseToken code);
uint8_t durationDits(char a);
void playScale(MorseToken code);
void txAlphabetSong(MorseToken code);
void txAlfaBravoCharlie(MorseToken code);
void txAlfaNNN(MorseToken code);
void txProm(MorseToken);
void setProm(MorseToken);

void askSum(MorseToken);
void gradeSum(MorseToken);

void askNext(MorseToken);
void askNextAlfa(MorseToken);
void gradeNext(MorseToken);

void txPrompt(MorseToken);
void setTime(MorseToken code);
void tx0to9(MorseToken);

// Fetch element i of the stack. The top element is s(0). No error checking
// is done.
MorseToken s(size_t i);
size_t symbolStackSize();
void pushSymbolStack(MorseToken code);
void popSymbolStack(MorseToken);
void exchangeStack(MorseToken);
void clearSymbolStack(MorseToken);
void txSymbolStackTop(MorseToken);
void txListSymbolStack(MorseToken);
void popN(uint8_t n);

void dispatchQCode(MorseToken);

void txAR(MorseToken);

void txEcho(MorseToken code);
void txGreeting(MorseToken code);
void getPressure(MorseToken code);
void txTick(MorseToken code);
void txTiTick(MorseToken code);
void txTock(MorseToken code);
void txMetronome(MorseToken);
void sumStack(MorseToken);
void guitarTune(MorseToken);

void setAlarm(MorseToken);

void updateUptime();
void setUptime(uint8_t u);
void txUptime(MorseToken);
