///\file actions.cpp Implementation of application-specific actions
#include "Arduino.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"
#include "settings.h"
#include "actions.h"
#include "mac.h"
#include "notes.h"
#include "sound.h"
#include "prosign.h"
#include "stack.h"
#include "teensy3_morse_bsp.h"

#include "Time.h"

const char* version("1.2");

const char* getVersion() {
  return version;
}

ActionFn actions[256];
ActionFn savedActions[256];

#define actionSize (sizeof actions/sizeof actions[0])

void doNothing(MorseToken) {}

void initActions() {
  for (size_t i(0); i < actionSize; ++i) {
    actions[i] = doNothing;
  }
}

uint8_t codeHash(MorseToken code) {
  return code >> (wordBits - CHAR_BIT);
}

void assignAction(MorseToken code, ActionFn action) {
  actions[codeHash(code)] = action;
}

void doAction(MorseToken code) {
  actions[codeHash(code)](code);
}

/// A little safeguard against smashing the saved copy of the actions
/// and not being able to get back to the primary commands.
bool restorePending(false);

void saveActions() {
  if (!restorePending) {
    restorePending = true;
    for (size_t i(0); i < actionSize; ++i) {
      savedActions[i] = actions[i];
    }
  }
}

void restoreActions() {
  if (restorePending) {
    for (size_t i(0); i < actionSize; ++i) {
      actions[i] = savedActions[i];
    }
    restorePending = false;
  } else {
    txError();
  }
}

void push1RestoreAll(MorseToken code) {
  pushSymbolStack(code);
  restoreActions();
}

void dispatchOnStack(MorseToken code) {
   pushSymbolStack(code);
   if (matchScore("V") == 255) {
      // Transmit the top of the stack.
      txSymbolStackTop(MorseToken());
   } else if (matchScore("QRS") == 255) {
      // Increase the dit time.
      slowDown();
      popN(3);
   } else if (matchScore("QRQ") == 255) {
      // Decrease the dit time.
      speedUp();
      popN(3);
   } else if (matchScore("QTR?") == 255) {
      // Ask the time.
      txTimeHHMM(MorseToken());
      popN(4);
   } else if (matchScore("QSR") == 255) {
      // Request identification.
      txString("ET1");
   } else if (matchScore("C") == 255) {
      // Clear the stack.
      txListSymbolStack(MorseToken());
      clearSymbolStack(MorseToken());
      Serial.println("Cleared stack.");
   } else if (matchScore("nA") == 255) {
      // Set the low voltage threshold.
      popN(1);
      uint32_t n(stackDigitsToUnsigned());
      const uint32_t hi(getMaximumVoltage() - getLowVoltageThresholdDefault());
      if (n - getLowVoltageThresholdDefault() < hi) {
         setLowVoltageThreshold(n);
         txString("K");
      } else {
         txError();
         txString("RANGE");
      }
   } else if (matchScore("nB") == 255) {
      // Set the high voltage threshold.
      popN(1);
      uint32_t n(stackDigitsToUnsigned());
      const uint32_t hi(getMaximumVoltage() - getHighVoltageThresholdDefault());
      if (n - getHighVoltageThresholdDefault() < hi) {
         setHighVoltageThreshold(n);
         txString("K");
      } else {
         txError();
         txString("RANGE");
      }
   } else if (matchScore("nD") == 255) {
      // Set the dit duration.
      popN(1);
      uint32_t n(stackDigitsToUnsigned());
      if (n - 20 <= 200 - 20) {
         setDitMicros(1000 * n);
         txString("K");
      } else {
         txError();
         txString("RANGE");
      }
   } else if (matchScore("F") == 255) {
      // Get the announcement format.
      popN(1);
      txString("TX MSG END WITH ^AR^  K");
      char buffer[128];
      rxString(buffer, sizeof buffer);
      txString("K");
      setAnnouncementFormat(buffer);
   } else if (matchScore("G") == 255) {
      // Copy the alarm format.
      popN(1);
      txString("TX MSG END WITH ^AR^  K");
      char buffer[128];
      rxString(buffer, sizeof buffer);
      txString("K");
      setAlarmLowFormat(buffer);
   } else if (matchScore("H") == 255) {
      // Copy the alarm format.
      popN(1);
      txString("TX MSG END WITH ^AR^  K");
      char buffer[128];
      rxString(buffer, sizeof buffer);
      txString("K");
      setAlarmHighFormat(buffer);
   } else if (matchScore("c^SS^") == 255) {
      // Erase the top of the stack.
      popN(2);
      txString("K");
   } else if (matchScore("nI") == 255) {
      // Set the nominal announcement repetition interval.
      popN(1);
      uint32_t n(stackDigitsToUnsigned());
      if (n - 20 < 86400 - 20) {
         setAnnouncementInterval(n);
         txString("K");
      } else {
         txError();
         txString("RANGE");
      }
   } else if (matchScore("nJ") == 255) {
      // Set the alarm announcement repetition interval.
      popN(1);
      uint32_t n(stackDigitsToUnsigned());
      if (n - 20 <= 86400 - 20) {
         setAlarmInterval(n);
         txString("K");
      } else {
         txError();
         txString("RANGE");
      }
   } else if (matchScore("nQTR") == 255 || matchScore("nT") == 255) {
      // Set the time without the four-digit requirement.
      popN(1);
      uint32_t n(stackDigitsToUnsigned());
      uint32_t hour(n / 100);
      uint32_t minute(n % 100);
      if (hour < 24 && minute < 60) {
         setTime(hour, minute, 0, 0, 0, 0);
         txString("K");
      } else {
         txError();
         txString("RANGE");
      }
   } else if (matchScore("nP") == 255) {
      // Set the pitch of the side tone.
      popN(1);
      uint32_t n(stackDigitsToUnsigned());
      if (n - 55 <= 1760 - 55) {
         setTxHz(n);
         txString("K");
      } else {
         txError();
         txString("RANGE");
      }
   } else if (matchScore("X") == 255) {
      // Exchange the top two elements of the stack.
      popN(1);
      exchangeStack(MorseToken());
      txString("K");
   } else if (matchScore("?") == 255) {
      // Transmit the help text.
      popN(1);
      txHelp(MorseToken());
   }
}

//----

void txSymbolStackTop(MorseToken) {
   txLiteral(s(0));
}

void txListSymbolStack(MorseToken) {
   Serial.print("Stack: ");
   for (size_t i(0); i < symbolStackSize(); ++i) {
      txLiteral(s(i));
      Serial.print(s(i).toChar());
   }
   Serial.println();
}

void txTimeHHMM(MorseToken code) {
  char buffer[] = "HHMM";
  unsigned int m(minute());
  unsigned int h(hour());
  buffer[0]  = '0' + h / 10;
  buffer[1]  = '0' + h % 10;
  buffer[2] = '0' + m / 10;
  buffer[3] = '0' + m % 10;
  txString(buffer);
}

void txTimeVerbose(MorseToken code) {
  //               0         1         2
  //               012345678901234567890
  char buffer[] = "QRT QRT HH MM ? HH MM";
  unsigned int m(minute());
  unsigned int h(hour());
  buffer[8]  = '0' + h / 10;
  buffer[9]  = '0' + h % 10;
  buffer[11] = '0' + m / 10;
  buffer[12] = '0' + m % 10;
  buffer[16] = buffer[8];
  buffer[17] = buffer[9];
  buffer[19] = buffer[11];
  buffer[20] = buffer[12];
  txString(buffer);
}

void txUptime(MorseToken) {
   Serial.print("uptime: ");
   if (getUptime() < 1000) Serial.print('0');
   if (getUptime() < 100) Serial.print('0');
   if (getUptime() < 10) Serial.print('0');
   Serial.println(getUptime());
   txUnsigned(getUptime());
}

void txInternalTemperatureC(MorseToken) {
  int degC(getInternalTemperatureC() + 0.5);
  txUnsigned(degC);
  Serial.println(degC);
}

void txInternalTemperatureF(MorseToken) {
  int degF(1.8 * getInternalTemperatureC() + 32.0);
  txUnsigned(degF);
  Serial.println(degF);
}

void txMacAddress(MorseToken) {
  read_mac();
  print_mac();
  for (uint8_t* p(mac); p < mac + 6; ++p) {
    uint8_t octet1(*p >> 4);
    uint8_t octet2(*p && 0x0F);
    txLetter("0123456789ABCDEF"[octet1]);
    txLetter("0123456789ABCDEF"[octet2]);
    delay(200);
  }
}



void slowDown() {
  Serial.println("QRS - slower");
  txString("QRS ");
  int d(getDitMicros());
  setDitMicros(d + 5000);
  txString("QRS");
}

void speedUp() {
  Serial.println("QRQ - faster");
  txString("QRQ ");
  int d(getDitMicros());
  if (25000 <= d) {
     setDitMicros(d - 5000);
     txString("QRQ");
  } else {
     txError();
     txString("RANGE");
  }
}

void txAR(MorseToken) {
  txString("^AR^ ^AR^ ^AR^");
}

void txEcho(MorseToken code) {
  txString("II "); // I say again.
  txLiteral(code);
  txString(" ^AR^"); // End of message.
}

void txGreeting(MorseToken code) {
  txString("CQ CQ CQ DE ET1 ET1 K");
  Serial.println();
}


void getPressure(MorseToken code) {
  txString("");
  pwmFrequency(0);
  pwmWrite(getDuty());
  while (!touchPoll(dahPin)) {
    uint32_t ci(touchRead(ditPin));
    Serial.println(ci);
    uint32_t f(map(ci, 500, 65535, 33, 4186));
    pwmFrequency(f);
    pwmWrite(getDuty());
    delayMicroseconds(getDitMicrosDefault());
  }
  txString("^AR^");
}

void txTick(MorseToken) {
  playTone(noteHz[96], getDitMicrosDefault()/2);
}

void txTiTick(MorseToken) {
  playTone(noteHz[98], getDitMicrosDefault()/4);
  delayMicroseconds(getDitMicrosDefault()/2);
  playTone(noteHz[98], getDitMicrosDefault()/4);
}

void txTock(MorseToken) {
  playTone(noteHz[84], getDitMicrosDefault()/2);
}

void printTouchThresholds() {
   char pinLabel2[] = "Pin xx (dit) touch capacitance threshold: ";
   if (ditPin < 10) {
      pinLabel2[4] = ditPin + '0';
      memmove(pinLabel2 + 5, pinLabel2 + 6, 37);
   } else {
      pinLabel2[4] = ditPin / 10 + '0';
      pinLabel2[5] = ditPin % 10 + '0';
   }
   printLabelValueUnits(pinLabel2, getPinThreshold(ditPin), " pF");

   if (ditPin < 10) {
      memmove(pinLabel2 + 6, pinLabel2 + 5, 36);
   }
   pinLabel2[9] = 'a';
   pinLabel2[10] = 'h';
   if (dahPin < 10) {
      pinLabel2[4] = dahPin + '0';
      memmove(pinLabel2 + 5, pinLabel2 + 6, 37);
   } else {
      pinLabel2[4] = dahPin / 10 + '0';
      pinLabel2[5] = dahPin % 10 + '0';
   }
   printLabelValueUnits(pinLabel2, getPinThreshold(dahPin), " pF");
}

/// Report settings and measurements over the serial port.
void printLog() {
   Serial.print(
      "Didah Voltage Monitor\r\n"
      "=====================\r\n");
   printLabelValueUnits("Version: ", getVersion(), "");
   Serial.print(
      "Documentation: http://pictographer.com/didah\r\n"
      "\r\n"
      "Device to periodically announce input voltage in Morse code.\r\n"
      "\r\n");

   Serial.print(
      "Measurements\r\n"
      "--------\r\n");

   //            0         1         2
   //            012345678901234567890
   char buffer[] = "HHMM";
   unsigned int m(minute());
   unsigned int h(hour());
   buffer[0]  = '0' + h / 10;
   buffer[1]  = '0' + h % 10;
   buffer[2] = '0' + m / 10;
   buffer[3] = '0' + m % 10;

   printLabelValueUnits("Time: ", buffer);

   // Last uptime over 24hr
   //!\todo not yet.
   uint32_t u(getUptime());
   // If this is still running 7656 years from now, send me a bug report
   // about the overflow.
   printLabelValueUnits("Uptime: ", 15 * u, " minutes");

   printLabelValueUnits("Measured voltage: ", readMillivolts(), " mV");
   printLabelValueUnits("Temperature: ", getInternalTemperatureC(), " C");

   Serial.println();

   Serial.print(
      "Settings\r\n"
      "--------\r\n");

   printLabelValueUnits(
      "Nominal announcement format: ", getAnnouncementFormat(), "");

   printLabelValueUnits(
      "Nominal announcement interval: ", getAnnouncementInterval(), " seconds");

   printLabelValueUnits(
      "Low-voltage alarm threshold: ", getLowVoltageThreshold(), " mV");

   printLabelValueUnits(
      "Low-voltage alarm announcement format: ", getAlarmLowFormat(), "");
  
   printLabelValueUnits(
      "High-voltage alarm threshold: ", getHighVoltageThreshold(), " mV");

   printLabelValueUnits(
      "High-voltage alarm announcement format: ", getAlarmHighFormat(), "");
  
   printLabelValueUnits(
      "Alarm interval: ", getAlarmInterval(), " seconds");

   printLabelValueUnits("Dit duration: ", getDitMicros()/1000, " ms");

   printLabelValueUnits("Output frequency: ", getTxHz(), " Hz");

   Serial.println();
}

void printBanner() {
   Serial.print(
      "Commands\r\n"
      "--------\r\n"
      "? - Help\r\n"
      "A - Set low-voltage alarm threshold in millivolts [2700-12000]\r\n"
      "B - Set high-voltage alarm threshold in millivolts [2700-12000]\r\n"
      "D - Set dit duration in milliseconds [20-200]\r\n"
      "F - Set normal announcement format\r\n"
      "    A-Z     literal character\r\n"
      "    0-9     literal character\r\n"
      "    <space> pause for one dit unit\r\n"
      "    <punctuation>\r\n"
      "            Literal punctuation: ");
   Serial.print(morsePunctuation);
   Serial.println(
      "\r\n"
      "    ^<ps>^  prosign of up to seven symbols.\r\n"
      "            For example prosign ^BK^ is transmitted -....-.\r\n"
      "    %V      monitor voltage in millivolts\r\n"
      "    %F      temperature in degrees Fahrenheit\r\n"
      "    %C      temperature in degrees Celsius\r\n"
      "    %T      time of day in four-digit 24-hour time: HHMM\r\n"
      "    %U      uptime in four digits: HHMM\r\n"
      "G - Set low-voltage announcement format\r\n"
      "H - Set high-voltage announcement format\r\n"
      "I - Set normal announcement interval in seconds [20-86400]\r\n"
      "J - Set alarm announcement interval in seconds [20-86400]\r\n"
      "L - Print settings and log\r\n"
      "O - Load settings from EEPROM\r\n"
      "P - Set output frequency (pitch) in Hz [55-1760]\r\n"
      "S - Save settings to EEPROM\r\n"
      "T - Set the 24-hour time [0000-2359]\r\n"
      );
}

//----

void setVolume(MorseToken) {
  size_t n(symbolStackSize());
  size_t dutyCycle(0);
  (2 < n) && (dutyCycle += 100 * s(2).m2i());
  (1 < n) && (dutyCycle +=  10 * s(1).m2i());
  (0 < n) && (dutyCycle +=   1 * s(0).m2i());
  popN(3 - (2 < n) - (1 < n));
  setDuty(dutyCycle);
}

bool readRange(uint32_t& result,
	       const char* prompt,
	       uint32_t minimum,
	       uint32_t maximum)
{
   Serial.print(prompt);
   uint32_t v;
   bool gotOne(readNumber(v));
   bool isInRange(v - minimum <= maximum - minimum);
   if (gotOne) {
      if (isInRange) {
         result = v;
      } else {
         Serial.print("\r\nOut of range. Value must be between ");
         Serial.print(minimum);
         Serial.print(" and ");
         Serial.print(maximum);
         Serial.println(".");
      }
   } else {
      Serial.println("\r\nInvalid number. Aborting.");
   }
   return gotOne && isInRange;
}

void readEdit(char* buffer, bool& isNotTooLate,
              const char* end, IntPredicate isIncluded, uint32_t timeout)
{
   const char ASCII_BS = '\b';
   const char ASCII_DEL = '\x7f';
   char* p(buffer);
   elapsedMicros elapsed;
   do {
      if (Serial.available()) {
         char in(Serial.read());
         if (isIncluded(in)) {
            *p = in;
            Serial.write(*p);
            ++p;
         } else if (in == '\r') {
            *p = 0;
            break;
         } else if (p != buffer && (in == ASCII_BS || in == ASCII_DEL)) {
            Serial.write(ASCII_BS); // back up one
            Serial.write(' ');      // erase by writing a space
            Serial.write(ASCII_BS); // back over the space
            *p = 0;
            --p;
         }
      }
      isNotTooLate = elapsed < timeout;
   } while (p != end && isNotTooLate);
   *p = 0;
}

bool readLine(char* buffer, size_t n, uint32_t timeout) {
  bool isNotTooLate(true);
  if (!n) {
    // No buffer space. You ask for nothing? You got it!
    return isNotTooLate;
  }
  const char* e(buffer + n - 1);

  readEdit(buffer, isNotTooLate, e, isprint, timeout);

  return isNotTooLate;
}

bool readNumber(uint32_t& v, uint32_t timeout) {
  bool isNotTooLate(true);
  char buffer[32];
  const char* e(buffer + sizeof buffer - 1);

  readEdit(buffer, isNotTooLate, e, isdigit, timeout);

  v = strtoul(buffer, 0, 0);
  return isNotTooLate;
}

void mapToUpper(char* buffer, size_t n) {
  for (size_t i(0); i < n; ++i) {
    buffer[i] = toupper(buffer[i]);
  }
}

bool isValidMorseFormat(const char* buffer, size_t n) {
  bool result(true);
  for (size_t i(0); i < n; ++i) {
    const char c(buffer[i]);
    if (ispunct(c)) {
      if (strchr(morsePunctuation, c)) {
	; // ok
      } else if (c == '%' && i + 1 < n && strchr("CFTUV", buffer[i + 1])) {
	; // ok
      } else if (c == '^') {
	; // ok, not checking prosign overflow, matched delimiters, or
	  // embedded escape sequences, e.g. 
	  //    ^^
          //    ^%F^
      } else {
	result = false;
	break;
      }
    } else if (!c) {
      break;
    }
  }
  return result;
}

bool readMessageFormat(char* buffer, size_t bufferSize, const char* prompt) {
   bool result(false);
   Serial.print(prompt);
   bool ok(readLine(buffer, bufferSize));
   if (ok) {
      mapToUpper(buffer, bufferSize);
      size_t inputLength(strlen(buffer));
      if (isValidMorseFormat(buffer, inputLength)) {
         if (inputLength) {
            Serial.print("\r\nFormat set to '");
            Serial.print(buffer);
            if (inputLength == bufferSize - 1) {
               Serial.print("'. (Truncated at ");
               Serial.print(bufferSize - 1);
               Serial.println(" characters.)");
            } else {
               Serial.println("'.");
            }
         } else {
            Serial.print("\r\nFormat set to default '");
            Serial.print(getAnnouncementFormatDefault());
            Serial.println("'.");
            strcat(buffer, getAnnouncementFormatDefault());
         }
         result = true;
      } else {
         Serial.println("\r\nInvalid character or sequence. Aborting.");
      }
   } else {
      Serial.println("\r\nKeyboard timer expired. Aborting.");
   }
   return result;
}

void readEvalPrint() {
   char cmd(toupper(Serial.read()));
   Serial.println(cmd);
   if (cmd == '\r') {
      Serial.print("> ");
   } else if (cmd == '?') {
      printLog();
      printBanner();
      Serial.print("> ");
   } else if (cmd == 'A') {
      uint32_t v;
      if (readRange(v,
                    "Enter low-voltage threshold in mV [2700-12000]: ",
                    getLowVoltageThresholdDefault(),
                    getMaximumVoltage()))
      {
         printLabelValueUnits("\r\nLow-voltage threshold: ", v, " mV");
         setLowVoltageThreshold(v);
      } 
      Serial.print("> ");
   } else if (cmd == 'B') {
      uint32_t v;
      if (readRange(v,
                    "Enter high-voltage threshold in mV [2700-12000]: ",
                    getHighVoltageThresholdDefault(),
                    getMaximumVoltage()))
      {
         printLabelValueUnits("\r\nLow-voltage threshold: ", v, " mV");
         setHighVoltageThreshold(v);
      } 
      Serial.print("> ");
   } else if (cmd == 'D') {
      uint32_t v;
      if (readRange(v, "Enter 'dit' duration in ms [20-200]: ", 20, 200)) {
         printLabelValueUnits("\r\nDit duration: ", v, " ms");
         setDitMicros(v * 1000);
      } 
      Serial.print("> ");
   } else if (cmd == 'F') {
      const size_t bufferSize(getAnnouncementBufferSize());
      (void) readMessageFormat(getAnnouncementBuffer(), bufferSize,
                               "Enter announcement format "
                               "[BAT %V MV ? %V MV]: ");
      Serial.print("> ");
   } else if (cmd == 'G') {
      const size_t bufferSize(getAlarmLowBufferSize());
      (void) readMessageFormat(getAlarmLowBuffer(), bufferSize,
                               "Enter low-voltage alarm format "
                               "[BAT %V MV ? %V MV]: ");
      Serial.print("> ");
   } else if (cmd == 'H') {
      const size_t bufferSize(getAlarmHighBufferSize());
      (void) readMessageFormat(getAlarmHighBuffer(), bufferSize,
                               "Enter high-voltage alarm format "
                               "[BAT %V MV ? %V MV]: ");
      Serial.print("> ");
   } else if (cmd == 'I') {
      uint32_t v;
      if (readRange(v, "Enter nominal announcement interval in "
                    "seconds [20-86400]: ", 20, 86400))
         {
            printLabelValueUnits("\r\nNominal announcement interval: ",
                                 v, " seconds");
            setAnnouncementInterval(v);
         } 
      Serial.print("> ");
   } else if (cmd == 'J') {
      uint32_t v;
      if (readRange(v, "Enter low-voltage alarm announcement interval in "
                    "seconds [20-86400]: ", 20, 86400))
         {
            printLabelValueUnits("\r\nAlarm announcement interval: ",
                                 v, " seconds");
            setAlarmInterval(v);
         } 
      Serial.print("> ");
   } else if (cmd == 'L') {
      printLog();
      Serial.print("> ");
   } else if (cmd == 'O') {
      initSettings();
      Serial.println("Settings restored.\r\n");
      printLog();
      Serial.print("> ");
   } else if (cmd == 'P') {
      uint32_t v;
      if (readRange(v, "Enter output frequency in Hz [55-1760]: ", 55, 1760)) {
         printLabelValueUnits("\r\nOutput frequency: ", v, " Hz");
         setTxHz(v);
      } 
      Serial.print("> ");
   } else if (cmd == 'R') {
      txRaw(MorseToken());
   } else if (cmd == 'S') {
      saveSettings();
      Serial.println("Settings saved.\r\n");
      Serial.print("> ");
   } else if (cmd == 'T') {
      uint32_t v;
      if (readRange(v, "Enter the 24-hour time [0000-2359]: ", 0, 2359)) {
         printLabelValueUnits("\r\nTime set to: ", v, "");
         setTime(v / 100, v % 100, 0, 0, 0, 0);
      } 
      Serial.print("> ");
   } else if (cmd == 'U') {
      Serial.print("hasRTC(): ");
      Serial.println(hasRTC() ? "yes" : "no");
      describeRTC();
      updateUptime();
      switch (timeStatus()) {
      case timeNotSet:
         Serial.println("timeNotSet");
         break;
      case timeNeedsSync:
         Serial.println("timeNeedsSync");
         break;
      case timeSet:
         Serial.println("timeSet");
         break;
      default:
         Serial.print("timeStatus() returned unexpected value: ");
         Serial.println(timeStatus());
      }
   } else {
      Serial.println("Unrecognized commmand. Enter '?' for help.");
      Serial.print("> ");
   }
   flushTouch();
}

void txHelp(MorseToken) {
   txString(
      "DIDAH ANNOUNCES VOLTAGE ON TX. "
      "SEE HTTP://PICTOGRAPHER.COM/DIDAH. "
      "USB SERIAL 9600 BAUD NO PARITY NO STOP BITS. "
      "OPEN CASE TO WIRE UP. "
      "MIN V 2700 MV. "
      "M FOR MENU. "
      "K");
}

void txMenu(MorseToken) {
   txString(
      "A  LOW V IN MV, "
      "B  HIGH V IN MV, "
      "C  CLEAR, "
      "D  DIT MS, "
      "F  MSG FMT, "
      "G  ALARM LOW FMT, "
      "H  ALARM HIGH FMT, "
      "I  RPT SECS, "
      "J  RPT ALARM SECS, "
      "L  LIST, "
      "M  MENU, "
      "O  LOAD, "
      "P  TONE HZ, "
      "S  SAVE, "
      "T  TIME, "
      "U  UPTIME. K"
      );
}

void txSettings(MorseToken) {
   txString("SETTINGS. "
            "MSG RPT SKED IS ");
   txUnsigned(getAnnouncementInterval());
   txString(" SECONDS. "
            "MSG FORMAT IS  ");

   char buf[128];
   {
      const char* a(getAnnouncementFormat());
      // Change % to ?. Do the reverse on data entry.
      strcpy(buf, a);
      char* p(buf);
      while (*p) {
         if (*p == '%') {
            *p = '?';
         }
         ++p;
      }
   }
   txString(buf);
   txString(" . "
            "ALARM LOW-VOLTAGE IS ");
   txUnsigned(getLowVoltageThreshold());
   txString(" MV. "
            "ALARM MSG RPT SKED IS ");
   txUnsigned(getAlarmInterval());
   txString(" SECONDS. "
            "ALARM LOW MSG FORMAT IS  ");
   {
      const char* a(getAlarmLowFormat());
      strcpy(buf, a);
      char* p(buf);
      while (*p) {
         if (*p == '%') {
            *p = '?';
         }
         ++p;
      }
   }
   txString(buf);
   txString("ALARM HIGH MSG FORMAT IS  ");
   {
      const char* a(getAlarmLowFormat());
      strcpy(buf, a);
      char* p(buf);
      while (*p) {
         if (*p == '%') {
            *p = '?';
         }
         ++p;
      }
   }
   txString(buf);
   txString(" .");
}

uint32_t getVoltagePollInterval() {
   return 30UL;
}

/// Voltage state: Low, Nominal, or High.  Just for fun, the Hamming
/// distance between these ASCII codes is selected to be greater than
/// 1.
enum alarmStateEnum {
   voltageLow = 'v',            // 111 0110
   voltageNominal = '-',        // 010 1101
   voltageHigh = '^'            // 101 1110
} alarmState;

void setAlarmState(char newState) {
   alarmState = static_cast<alarmStateEnum>(newState);
}

char getAlarmState() {
   return alarmState;
}

uint32_t getIntervalForState() {
   uint32_t result;
   if (alarmState != voltageNominal) {
      result = getAlarmInterval();
   } else {
      result = getAnnouncementInterval();
   }
   return result;
}

const char* getFormatForState() {
   switch (alarmState) {
   case voltageLow:     return getAlarmLowFormat();
   case voltageNominal: return getAnnouncementFormat();
   case voltageHigh:    return getAlarmHighFormat();
   default:
      txError();
      Serial.println("INTERNAL ERROR: Undefined state in getFormatForState().");
      return 0;
   }
}

void txRaw(MorseToken) {
   Serial.println(getRaw());
}
