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

const int32_t maximumVoltage = 12000;

const int32_t getMaximumVoltage() {
   return maximumVoltage;
}

const char* getDefaultAnnouncementFormat() {
   return defaultAnnouncementFormat;
}

/// Default announcement interval in seconds.
const uint32_t defaultAnnouncementInterval = 600;

/// Default alarm interval in seconds.
const uint32_t defaultAlarmInterval = 30;

/// Default transmission frequency in Hz.
const uint32_t defaultFrequency = 750;

struct {
   //!\todo Number of 15-minute intervals since something
   uint32_t uptime;

   /// Low-voltage alert threshold in millivolts.  Note: if the
   /// low-voltage threshold is lower than the transmitter's minimum
   /// voltage, or lower than the microprcessor's minimum voltage, the
   /// alarm will not be transmitted.
   int32_t lowVoltageThreshold = lowVoltageThresholdDefault;

   /// Duration of a 'dit' in microseconds
   uint32_t ditMicros = ditMicrosDefault;

   /// Format for periodic announcements in an application-specific syntax
   char announcementFormat[128] = defaultAnnouncementFormat;

   /// Format for periodic announcements when voltage is low
   char alarmFormat[128] = defaultAnnouncementFormat;

   /// Repeat the announcement when the time in seconds is a multiple
   /// of this number.
   uint32_t announcementInterval = defaultAnnouncementInterval;

   /// Repeat the low-voltage alarm when the time in seconds is a
   /// multiple of this number.
   uint32_t alarmInterval = defaultAlarmInterval;

   /// Initial frequency of the 'carrier wave' tones.
   uint32_t txHz = defaultFrequency;
} settings;

size_t getAnnouncementBufferSize() {
   return sizeof settings.announcementFormat;
}

char* getAnnouncementBuffer() {
   return settings.announcementFormat;
}

size_t getAlarmBufferSize() {
   return sizeof settings.alarmFormat;
}

char* getAlarmBuffer() {
   return settings.alarmFormat;
}

uint32_t getTxHz() {
  return settings.txHz;
}

void setTxHz(uint32_t Hz) {
  settings.txHz = Hz;
}

void initSettings() {
   (void) EEPROM_readAnything(0, settings);
}

void saveSettings() {
   (void) EEPROM_writeAnything(0, settings);
}

/// Write to flash so that next time we start, so we can report
/// how long the previous run was.
void updateUptime() {
   //    EEPROM.write(0, ++uptime);
   //  eeprom_write_dword((uint32_t *) 0, );
   ++settings.uptime;
   Serial.print("Updated time: ");
   Serial.println(settings.uptime);
}

void setUptime(uint32_t u) {
   settings.uptime = u;
}

uint32_t getUptime() {
   return settings.uptime;
}
  
void setLowVoltageThreshold(int32_t v) {
   settings.lowVoltageThreshold = v;
}

int32_t getLowVoltageThreshold() {
   return settings.lowVoltageThreshold;
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

void setAlarmFormat(const char* format) {
   const size_t n(sizeof settings.alarmFormat);
   strncpy(settings.alarmFormat, format, n);
   settings.alarmFormat[n - 1] = 0;
}

const char* getAnnouncementFormat() {
  return settings.announcementFormat;
}

const char* getAlarmFormat() {
  return settings.alarmFormat;
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

