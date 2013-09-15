/// Implementation of user settings

///\file settings.cpp

#include "EEPROMAnything.h"

#define defaultAnnouncementFormat "BAT %V MV ? %V MV"

/// Duration of a dit in microseconds
const uint32_t ditMicrosDefault = 40000;

const int32_t lowVoltageThresholdDefault = 2700;

const int32_t getLowVoltageThresholdDefault() {
   return lowVoltageThresholdDefault;
}

const int32_t maximumVoltage = 16000;

const int32_t getMaximumVoltage() {
   return maximumVoltage;
}

const int32_t highVoltageThresholdDefault = maximumVoltage;

const int32_t getHighVoltageThresholdDefault() {
   return highVoltageThresholdDefault;
}

const char* getAnnouncementFormatDefault() {
   return defaultAnnouncementFormat;
}

/// Default announcement interval in seconds.
const uint32_t defaultAnnouncementInterval = 600;

/// Default alarm interval in seconds.
const uint32_t defaultAlarmInterval = 30;

/// Default transmission frequency in Hz.
const uint32_t defaultFrequency = 750;

struct {
   /// Low-voltage alert threshold in millivolts.  Note: if the
   /// low-voltage threshold is lower than the transmitter's minimum
   /// voltage, or lower than the microprcessor's minimum voltage, the
   /// alarm will not be transmitted.
   int32_t lowVoltageThreshold = lowVoltageThresholdDefault;

   /// High-voltage alert threshold in millivolts.  Note: the hardware
   /// has been tested briefly at 19.6V, but the design target is a
   /// 12V battery. The theoretical maximum input voltage is
   /// approximately 34V based on the voltage divider, the ADC
   /// maximum input voltage of 3.3V, and the reverse voltage
   /// protection diode, but that doesn't account for thermal stress.
   ///
   /// The voltage divider values are 993 KOhm and 107.5 KOhm.
   int32_t highVoltageThreshold = highVoltageThresholdDefault;

   /// Duration of a 'dit' in microseconds
   uint32_t ditMicros = ditMicrosDefault;

   /// Format for periodic announcements in an application-specific syntax
   char announcementFormat[128] = defaultAnnouncementFormat;

   /// Format for periodic announcements when voltage is low
   char alarmLowFormat[128] = defaultAnnouncementFormat;

   /// Format for periodic announcements when voltage is low
   char alarmHighFormat[128] = defaultAnnouncementFormat;

   /// Repeat the announcement when the time in seconds is a multiple
   /// of this number.
   uint32_t announcementInterval = defaultAnnouncementInterval;

   /// Repeat the low-voltage alarm when the time in seconds is a
   /// multiple of this number.
   uint32_t alarmInterval = defaultAlarmInterval;

   /// Initial frequency of the 'carrier wave' tones.
   uint32_t txHz = defaultFrequency;
} settings;

/// Number of 15-minute intervals since power on. The value gets
/// written to EEPROM when it exceeds 2 (30 minutes) so that it is
/// possible to see how long the previous uptime was.
uint32_t uptime;

size_t getAnnouncementBufferSize() {
   return sizeof settings.announcementFormat;
}

char* getAnnouncementBuffer() {
   return settings.announcementFormat;
}

size_t getAlarmLowBufferSize() {
   return sizeof settings.alarmLowFormat;
}

char* getAlarmLowBuffer() {
   return settings.alarmLowFormat;
}

size_t getAlarmHighBufferSize() {
   return sizeof settings.alarmHighFormat;
}

char* getAlarmHighBuffer() {
   return settings.alarmHighFormat;
}

uint32_t getTxHz() {
  return settings.txHz;
}

void setTxHz(uint32_t Hz) {
  settings.txHz = Hz;
}

void initSettings() {
   (void) EEPROM_readAnything(4, settings);
}

void saveSettings() {
   (void) EEPROM_writeAnything(4, settings);
}

bool timeIsSet(false);

bool isTimeSet() {
   return timeIsSet;
}

elapsedMicros sinceUptime;

void updateUptime() {
   if (15 * 60 * 1000000 <= sinceUptime) {
      ++uptime;
      (void) EEPROM_writeAnything(0, uptime);
      sinceUptime = 0;
   }
}

void setUptime(uint32_t u, bool initializing) {
   timeIsSet = !initializing;
   uptime = u;
}

uint32_t getUptime() {
   return uptime;
}
  
void setLowVoltageThreshold(int32_t v) {
   settings.lowVoltageThreshold = v;
}

int32_t getLowVoltageThreshold() {
   return settings.lowVoltageThreshold;
}

void setHighVoltageThreshold(int32_t v) {
   settings.highVoltageThreshold = v;
}

int32_t getHighVoltageThreshold() {
   return settings.highVoltageThreshold;
}

//---- Morse Code ----

uint32_t getDitMicrosDefault() {
   return ditMicrosDefault;
}

void setDitMicros(uint32_t ms) {
   settings.ditMicros = ms;
}

uint32_t getDitMicros() {
  return settings.ditMicros;
}

void setAnnouncementFormat(const char* format) {
   const size_t n(sizeof settings.announcementFormat);
   strncpy(settings.announcementFormat, format, n);
   settings.announcementFormat[n - 1] = 0;
}

void setAlarmLowFormat(const char* format) {
   const size_t n(sizeof settings.alarmLowFormat);
   strncpy(settings.alarmLowFormat, format, n);
   settings.alarmLowFormat[n - 1] = 0;
}

void setAlarmHighFormat(const char* format) {
   const size_t n(sizeof settings.alarmHighFormat);
   strncpy(settings.alarmHighFormat, format, n);
   settings.alarmHighFormat[n - 1] = 0;
}

const char* getAnnouncementFormat() {
  return settings.announcementFormat;
}

const char* getAlarmLowFormat() {
  return settings.alarmLowFormat;
}

const char* getAlarmHighFormat() {
  return settings.alarmHighFormat;
}

void setAlarmInterval(uint32_t seconds) {
  settings.alarmInterval = seconds;
}

void setAnnouncementInterval(uint32_t seconds) {
  settings.announcementInterval = seconds;
}

uint32_t getAnnouncementInterval() {
  return settings.announcementInterval;
}

uint32_t getAlarmInterval() {
  return settings.alarmInterval;
}

