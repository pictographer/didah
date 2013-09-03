///\file pda.cpp
///\brief Implementation of Personal Digital Assistant functions
#include "pda.h"
#include "notes.h"
#include "settings.h"
#include "actions.h"
#include "stack.h"
#include "sound.h"
#include "prosign.h"
#include "EEPROMAnything.h"
#include "Time.h"

// Homeless PDA stuff

// We're past this. Echoing do-nothing characters isn't so useful.
//   for (char c('A'); c <= 'Z'; ++c) {
//      assignAction(a2m(c), txEcho);
//   }
//   assignAction(a2m('D'), playNote);
//   assignAction(a2m('F'), txInternalTemperatureC);
//   assignAction(a2m('G'), guitarTune);
//   assignAction(a2m('H'), askSum);
//   assignAction(a2m('H'), txHelp);
//   assignAction(a2m('I'), gradeSum);
//   assignAction(a2m('J'), askNextAlfa);
//   assignAction(a2m('L'), txListSymbolStack);
//   assignAction(a2m('P'), popSymbolStack);
//   assignAction(a2m('K'), doMatchScore);
//   assignAction(a2m('M'), txMetronome);
//   assignAction(a2m('N'), tx0to9);
//   assignAction(a2m('+'), sumStack);
//   assignAction(a2m('W'), playScale);
//   assignAction(a2m('Y'), getPressure);
//   assignAction(a2m('Z'), playSoundEffect);

//    //!\todo needs to be longer! Dangerous.
//    assignAction(a2m('N'), setVolume);
//    //  assignAction(a2m('X'), exchangeStack);


//
// "QTR?" - What is the correct time?
// "n QRQ" - Send faster at (n words per minute)
// "n QRS" - Send slower at (n words per minute)
// "QRZ?" - By whom am I being called?
// "w QRZ" - You are being called by (w)
// "QRU?" - Do you have anything for me? (What is the next alarm message?)
// "QRU" - I have nothing.
// "QSD" - Your keying is incorrect; your signals are bad.
// "n QSG" - Transmit (n) telegrams at a time.
// "QSL?" - Can you acknowledge receipt
// "QSL" - Acknowledging receipt
// "QSM?" - Shall I repeat the last telegram?
// "QSM" - Repeat the last telegram.
// "n QSZ" - Send each word or group (n) times
// "n QTC" - I have (n) telegrams to send.
// "QTH?" - What is your position?
// "n QTA" - Erase telegram (n)
// "QTB?" - Do you agree with my word count?
// "QTB" - I do not agree; I shall repeat the first letter or first figure
//         of each

//!\todo
//   txGreeting(MorseToken());
//   } else if (matchScore("ddA") == 255) {
//      txAlfaBravoCharlie(MorseToken());
//
// I can't remember what this was about!
//    if (matchScore("AVR") == 255) {
//       setSenseMode(true);
//    } else if (matchScore("AVS") == 255) {
//       setSenseMode(false);
//    } else 

// This is more relevant to a PDA or device that routinely takes 
// user input as opposed to occasional configuration/interrogation.

// Process a few Q codes.
//    QRS - slow down
//    QRQ - speed up
//    QTR? - what is the time
//    QSR - please repeat your call (XXX Would QSM be better?)

//    } else if (matchScore("ddddQRX") == 255) {
//       Serial.println("Set time");
//       // Set an alarm for HH:MM.
//       popN(3);
//       setAlarm(MorseToken());
//    } else if (matchScore("QRX?") == 255) {
//       // Report alarm time or N for no alarm.
//    } else if (matchScore("dQSM") == 255) {
//       // Repeat telegram d
//       //!\todo an M wouldn't get here because it isn't assigned this action.
//    } else if (matchScore("dQTA") == 255) {
//       // Cancel telegram d
//    } else if (matchScore("dQTC") == 255) {
//       // Await d telegrams
//       // dQRJ - Await d words
//       // _CT_ means 'start copying'
//       // _AR_ or + means 'end of message'
//    } else if (matchScore("QTC?") == 255) {
//       // Report number of stored telegrams
//    } else if (matchScore("ABC") == 255) {
//       txAlphabetSong(MorseToken());

// Probably won't need this. The EEPROMAnything.h is better.
// void check_read_dword() {
//   const uint32_t *addr(0);
//   uint32_t e(eeprom_read_dword(addr));
// }

void pressureTone(uint16_t duration) {
  elapsedMicros t(0);
  while (t < duration) {
    for (int i(1); i < 50; ++i) {
      dWrite(HIGH);
      delayMicroseconds((500 + random(log(touchRead(ditPin)))));
      dWrite(LOW);
      delayMicroseconds((500 + random(log(touchRead(ditPin)))));
    }
  }
}

/// Play a chromatic scale.
void playScale(MorseToken) {
  analogWrite(piezoTxN, 0);
  for (size_t i(24); i < sizeof(noteHz)/sizeof(noteHz[0]); i += 1) {
    pwmFrequency(noteHz[i]+0.5);
    pwmWrite(getDuty());
    delayMicroseconds(getDitMicros());
    pwmWrite(0);
  }
}

/// Play one of 10 sound effects based on the digit on the stack.
void playSoundEffect(MorseToken code) {
  pwmFrequency(noteHz[48]+0.5);
  pwmWrite(getDuty());
  delayMicroseconds(getDitMicros());
  pwmWrite(0);
  if (symbolStackSize()) {
    uint8_t n(s(0).m2i());
    elapsedMicros t;
    size_t steps[] = { 0, 2, 3, 5, 7, 8, 10 };
    switch (n) {
    case 0:
      t = 0;
      // Low pulsing static rumble
      while (t < 60 * getDitMicrosDefault()) {
	for (int i(1); i < 25; ++i) {
	  dWrite(HIGH);
	  delayMicroseconds((1 + random(i)) * 1136);
	  dWrite(LOW);
	  delayMicroseconds((1 + random(i)) * 1136);
	}
      }
      break;
    case 1:
      shufflingCash(120 * getDitMicrosDefault());
      break;
    case 2:
      t = 0;
      // Descending beats
      while (t < 120 * getDitMicrosDefault()) {
	for (int i(1); i < 75; ++i) {
	  dWrite(HIGH);
	  delayMicroseconds(i * 1136/12);
	  dWrite(LOW);
	  delayMicroseconds(i * 1136/12);
	}
      }
      break;
    case 3:
      t = 0;
      // Buzzer
      while (t < 60 * getDitMicrosDefault()) {
	playTone(noteHz[36], getDitMicrosDefault()/30);
	playTone(noteHz[48], getDitMicrosDefault()/30);
	playTone(noteHz[52], getDitMicrosDefault()/60);
	playTone(noteHz[57], getDitMicrosDefault()/60);
      }
      break;
    case 4:
      t = 0;
      // Humm, kinda like a machine hum.
      while (t < 60 * getDitMicrosDefault()) {
	for (int i(0); i < 49; ++i) {
	  playTone(noteHz[i], getDitMicrosDefault()/10);
	}
      }
      break;
    case 5:
      t = 0;
      // ascending and repeating 
      while (t < 60 * getDitMicrosDefault()) {
	for (int i(0); i < 49; ++i) {
	  playTone(noteHz[i], getDitMicrosDefault()/2);
	}
      }
      break;
    case 6:
      t = 0;
      // ascending more quickly
      while (t < 60 * getDitMicrosDefault()) {
	for (int i(0); i < 49; ++i) {
	  playTone(noteHz[random(1,96)], getDitMicrosDefault()/10);
	}
      }
      break;
    case 7:
      t = 0;
      // ?
      while (t < 60 * getDitMicrosDefault()) {
	for (int i(0); i < 7; ++i) {
	  playTone(noteHz[(24 + steps[i % 7]) % 97], getDitMicrosDefault());
	}
      }
      break;
    case 8:
      t = 0;
      // Almost white noise, like water from a tap
      while (t < 60 * getDitMicrosDefault()) {
	dWrite(HIGH);
	delayMicroseconds(random(100,1000));
	dWrite(LOW);
	delayMicroseconds(random(100,1000));
      }
      break;
    case 9:
      t = 0;
      // Almost white noise, like water from a tap
      while (t < 60 * getDitMicrosDefault()) {
	dWrite(HIGH);
	delayMicroseconds(random(10,8000));
	dWrite(LOW);
	delayMicroseconds(random(10,8000));
      }
      t = 0;
      // Sort of like a pinball machine
      { int i(2);
	while (t < 60 * getDitMicrosDefault()) {
	  for (int k(0); k < 10; ++k) {
	    dWrite(HIGH);
	    delayMicroseconds(i);
	    dWrite(LOW);
	    delayMicroseconds(i);
	  }
	  i <<= 2;
	  if (4000 < i) { i = 2; }
	}
      }
    }
    txString("K");
  } else {
    txError();
  }
}

/// If the top two elements of the stack are digits, convert them to a
/// number between 0-99 inclusive. Play the corresponding note.
void playNote(MorseToken) {
  if (1 < symbolStackSize() && isdigit(s(0).toChar()) && isdigit(s(1).toChar())) {
    size_t note(10 * (s(1).toChar() - '0') + (s(0).toChar() - '0'));
    playTone(noteHz[note] + 0.5, getDitMicros() * 60);
    txUnsigned(note);
  } else {
    txError();
  }
}

/// Return the duration of the Morse code for the given character in
/// dits. E.g. an E would return 2; and an N would return 6.
uint8_t durationDits(char a) {
   if (a == ' ') {
      return wordGapFactor;
   } else {
      uint8_t code(MorseToken(a, MorseToken::Char));
      uint8_t count(0);
      while (code != MORSE_MSB) {
         count += ((code & MORSE_MSB) ? dahFactor : ditFactor) + ditFactor;
         code <<= 1;
      }
      return count;
   }
}

/// Play the A.B.C's with each single syllable letter replaced by the
/// corresponding Morse code. W is transmitted once for each syllable.
/// After Z, the remainder of the song is played as simple tones.
void txAlphabetSong(MorseToken) {
  uint32_t saveHz(getTxHz());
  int saveDitMicros(getDitMicros());
  const char* letters("ABCDEFGHIJKLMNOPQRSTUVWWWXY Z ");
  const char*
    notes("C42C42G42G42" "A52A52G44" "F42F42E42E42" "D41D41D41D41C44"
	  "G42G42F44"    "E42E42D44" "G41G41G42F44" "E42E42D44"
	  "C42C42G42G42" "A52A52G44" "F42F42E42E42" "D42D42C44");
  char* l(const_cast<char*>(letters));
  char* p(const_cast<char*>(notes));
  while (*p) {
    size_t steps[] = { 0, 2, 3, 5, 7, 8, 10 };
    size_t note(steps[p[0] - 'A'] + 12 * (p[1]-'0'));
    size_t frequency(noteHz[note] + 0.5);
    size_t duration(saveDitMicros * (p[2] - '0') * 2 / 3);
    char buffer[] = { '\0', ' ', '\0' };
    buffer[0] = l[0];
    if (l[1]) {
      setTxHz(frequency);
      setDitMicros(duration);
      txString(buffer);
      delayMicroseconds((14 * saveDitMicros + 1) -
			(saveDitMicros * durationDits(l[0])));
      ++l;
    } else {
      // This part goes much faster, but it doesn't convey much
      // information.
      playTone(frequency, duration);
    }
    p += 3;
  }
  setTxHz(saveHz);
  setDitMicros(saveDitMicros);
}

const char* alphaBravoCharlie("ALFA BRAVO CHARLIE DELTA "
			      "ECHO FOXTROT GOLF HOTEL "
			      "INDIA JULIETT KILO LIMA "
			      "MIKE NOVEMBER OSCAR PAPA "
			      "QUEBEC ROMEO SIERRA TANGO "
			      "UNIFORM VICTOR WHISKEY "
			      "X-RAY YANKEE ZULU ");

void txAlfaBravoCharlie(MorseToken code) {
   //  uint8_t n(m2a(code) - 'A');
   uint8_t n(0);
   if (1 < symbolStackSize()) {
      n = 10 * s(1).m2i() + s(0).m2i();
      popN(2);
   } else if (symbolStackSize()) {
      n = m2i(s(0));
      popN(1);
   }
   if (0 <= n && n <= ('Z' - 'A')) {
      char letterName[1 + sizeof "NOVEMBER"];
      const char* p(alphaBravoCharlie);
      size_t i(0);
      while (*p) {
         if (*p == ' ') {
            if (n) {
               --n;
            } else {
               break;
            }
         } else if (!n) {
            letterName[i++] = *p;
         }
         ++p;
      }
      letterName[i] = '\000';
      txString(letterName);
   } else {
      txError();
      Serial.println("txAlfaBravoCharlie: Range error.");
   }
}

/// This is a little harder than askNext in that the extra letters can
/// be distracting, but it is a little easier because there's more
/// context. I expect that for beginners, the former is easier, but for
/// intermediates, the latter is good practice.
void askNextAlfa(MorseToken) {
   uint8_t a(random(26));
   txAlfaBravoCharlie(MorseToken(a + 'A', MorseToken::Char));
   uint8_t b((a + 1) % 26);
   pushSymbolStack(MorseToken(b + 'A', MorseToken::Char));
   assignAction(MorseToken('I', MorseToken::Char), gradeNext);
   saveActions();
   for (size_t i('A'); i <= 'Z' ; ++i) {
      assignAction(MorseToken(i, MorseToken::Char), push1RestoreAll);
   }
}

/// Just like txAlfaBravoCharlie, but adds digits from
/// the stack to expose the entire alphabet.
void txAlphaNNN(MorseToken) {
  uint8_t n(0);
  uint8_t k(symbolStackSize());
  if (0 < k) {
    n += s(0).m2i();
    popSymbolStack(MorseToken());
  }
  if (1 < k) {
    n += 10 * s(1).m2i();
    popSymbolStack(MorseToken());
  }
  if (0 <= n && n <= ('Z' - 'A')) {
    char letterName[1 + sizeof "NOVEMBER"];
    const char* p(alphaBravoCharlie);
    size_t i(0);
    while (*p) {
      if (*p == ' ') {
	if (n) {
	  --n;
	} else {
	  break;
	}
      } else if (!n) {
	letterName[i++] = *p;
      }
      ++p;
    }
    txString(letterName);
  } else {
    txError();
  }
}

/// Note: Since txTick() takes a significant amount of time, the
/// fastest the metronome will currently go is 240 bpm.
void txMetronome(MorseToken) {
  uint8_t bpm(60);
  switch (symbolStackSize()) {
  case 3: 
    bpm = 100 * s(2).m2i() + 10 * s(1).m2i() + s(0).m2i();
    popN(3);
    break;
  case 2: 
    bpm = 10 * s(1).m2i() + s(0).m2i();
    popN(2);
    break;
  case 1:
    bpm = s(0).m2i();
    popN(1);
    break;
  default: txError();
  }
  uint32_t timeout(60 * 1000000 / bpm);
  elapsedMicros sinceTick(0);
  while (!touchPoll(dahPin)) {
    if (timeout < sinceTick) {
      sinceTick = 0;
      txTick(MorseToken());
    }
  }
}

void sumStack(MorseToken) {
  int sum(0);
  for (size_t i(0); i < symbolStackSize(); ++i) {
    sum += s(i).m2i();
  }
  txUnsigned(sum);
}

void guitarTune(MorseToken) {
   uint32_t duration(getDitMicros() * 6);
   // EADGBE
   playTone(noteHz[40] + 0.5, duration);
   playTone(noteHz[45] + 0.5, duration);
   playTone(noteHz[50] + 0.5, duration);
   playTone(noteHz[55] + 0.5, duration);
   playTone(noteHz[59] + 0.5, duration);
   playTone(noteHz[64] + 0.5, duration);
}

//---- Games ----

/// Histogram of digits for which the user made a mistake
uint8_t digitErr[10];

/// Start the error count at the maximum index of digitErr[] so that a
/// random ordinal in the range (0..digitalErrorCount) covers
/// digitErr[].
uint16_t digitErrCount(9);

/// Generate a random digit with the probability distribution implied
/// by the digitErr histogram. 
//
/// The sum of the entries in digitErr always equals digitErrCount. A
/// random ordinal number in the range (0..digitErrCount) is compared
/// to the cumulative distribution (calculated dynamically from the
/// ordinary histogram. As long as the random number is less than the
/// cumulative distribution, the search continues.
uint8_t getDigitByWeight() {
  uint8_t a(random(digitErrCount));
  uint8_t result(9);
  for (uint8_t i(0), sum(0); i < 10; ++i) {
    sum += digitErr[i] + 1;
    if (a < sum) {
      result = i;
      break;
    }
  }
  return result;
}

void gradeRestore(MorseToken answer) {
  pushSymbolStack(answer);
  gradeSum(MorseToken());
  restoreActions();
}

/// Present a simple challenge consisting of two random digits to 
/// add. The user tries to enter the last digit of their sum.
void askSum(MorseToken) {
   uint8_t a(getDigitByWeight());
   uint8_t b(getDigitByWeight());
   char buffer[3] = { char(a + '0'), char(b + '0'), 0 };
   txString(buffer);
   pushSymbolStack(MorseToken(buffer[0], MorseToken::Char));
   pushSymbolStack(MorseToken(buffer[1], MorseToken::Char));
   saveActions();
   for (size_t i('0'); i <= '9' ; ++i) {
      assignAction(MorseToken(i, MorseToken::Char), gradeRestore);
   }
}

/// The top three entries on the stack are two digits to add and their
/// proported sum mod 10. Transmit Y/N to indicate whether or not the
/// sum is correct. If the sum is not correct, transmit the correct
/// sum. Update the weights to bias future random number selection.
void gradeSum(MorseToken) {
  if (2 < symbolStackSize()) {
    uint8_t s1(s(1).m2i());
    uint8_t s2(s(2).m2i());
    uint8_t s0(s(0).toChar());
    uint8_t sum((s1 + s2) % 10);
    Serial.print(s2);
    Serial.print(" ");
    Serial.print(s1);
    Serial.print(" ");
    Serial.print(s0);
    Serial.print(" sum: ");
    Serial.println(sum);
    bool correct(s(0).m2i() == sum);
    if (correct) {
      txString("Y");
      // Decrease the error counts associated with the digits the user
      // just added incorrectly, taking care not to underflow.
      uint8_t d1(!!digitErr[s1]);
      uint8_t d2(!!digitErr[s2]);
      digitErr[s1] -= d1; 
      digitErr[s2] -= d2;
      digitErrCount -= (d1 + d2);
      Serial.print("Decrementing ");
      Serial.print(s1);
      Serial.print(". New weight is: ");
      Serial.println(digitErr[s1]);
      Serial.print("Decrementing ");
      Serial.print(s2);
      Serial.print(". New weight is: ");
      Serial.println(digitErr[s2]);
    } else {
      //            012345678901
      char buffer[] = "N  a + b = c";
      buffer[3] = s2 + '0';
      buffer[7] = s1 + '0';
      buffer[11] = sum + '0';
      txString(buffer);
      // Increase the error counts associated with the digits the user
      // just added incorrectly, taking care not to overflow.
      uint8_t d1(3 * (digitErr[s1] - 3 < 255));
      uint8_t d2(3 * (digitErr[s2] - 3 < 255));
      digitErr[s1] += d1;
      digitErr[s2] += d2;
      digitErrCount += d1 + d2;
      Serial.print("Incrementing ");
      Serial.print(s1);
      Serial.print(". New weight is: ");
      Serial.println(digitErr[s1]);
      Serial.print("Incrementing ");
      Serial.print(s2);
      Serial.print(". New weight is: ");
      Serial.println(digitErr[s2]);
    }
    // If the user plays long enough without a reset to saturate two
    // of the digits chosen at random, divide all the error counts
    // by 2.
    if (digitErr[s1] == 255 && digitErr[s2] == 255) {
      for (uint8_t i(0); i < 10; ++i) {
	digitErr[i] <<= 1;
      }
      digitErrCount <<= 1;
      Serial.println("Halving all weights.");
    }
    Serial.print("Digit error count is: ");
    Serial.println(digitErrCount);
    popN(3);
  } else {
    txError();
  }
}

/// Prompt with a random letter of the alphabet. The user enters the
/// letter then enters I to get graded.
void askNext(MorseToken) {
   uint8_t a(random(26));
   char buffer[3] = { char(a + 'A'), 0 };
   txString(buffer);
   uint8_t b((a + 1) % 26);
   pushSymbolStack(MorseToken(b + 'A', MorseToken::Char));
   assignAction(MorseToken('I', MorseToken::Char), gradeNext);
   saveActions();
   for (size_t i('A'); i <= 'Z' ; ++i) {
      assignAction(MorseToken(i, MorseToken::Char), push1RestoreAll);
   }
}

void gradeNext(MorseToken) {
   if (1 < symbolStackSize()) {
      if (s(1) == s(0)) {
         txString("Y");
         popSymbolStack(MorseToken());
         popSymbolStack(MorseToken());
      } else {
         txString("N");
         popSymbolStack(MorseToken());
         txSymbolStackTop(MorseToken());
         popSymbolStack(MorseToken());
      }
   } else {
      txError();
   }
}

