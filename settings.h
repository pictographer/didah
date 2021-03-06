/// User settings
// -*- mode: C++; -*-
///\file settings.h
#pragma once

/// Enable the radio transmitter for 120 ms before transmitting.
const uint32_t getRadioStartupDelay();

/// The default low-voltage threshold is the minimum voltage.
const int32_t getLowVoltageThresholdDefault();

/// The default high-voltage threshold is the maximum voltage.
const int32_t getHighVoltageThresholdDefault();

/// Get the maximum voltage the appliance is designed to accept. Returns
/// 16000 mV.
const int32_t getMaximumVoltage();

/// Returns the message format template
///    `BAT %V MV ? %V MV`
/// See txString() for a description of the format parameters such as %V.
const char* getAnnouncementFormatDefault();

/// Returns a pointer to the announcement buffer template. Used to update
/// the format.
char* getAnnouncementBuffer();

/// Returns the buffer size so updates can be done safely.
size_t getAnnouncementBufferSize();

/// Returns a pointer to the alarm buffer template. Used to update
/// the format.
char* getAlarmLowBuffer();

/// Returns the buffer size so updates can be done safely.
size_t getAlarmLowBufferSize();

/// Returns a pointer to the alarm buffer template. Used to update
/// the format.
char* getAlarmHighBuffer();

/// Returns the buffer size so updates can be done safely.
size_t getAlarmHighBufferSize();

/// Load the settings from non-volatile memory.
void initSettings();

/// Save the settings to non-volatile memory.
void saveSettings();

/// Get the transmission frequency (side tone).
uint32_t getTxHz();

/// Set the transmission frequency (side tone).
void setTxHz(uint32_t Hz);

///\brief If 15 minutes have elapsed since last time, write to EEPROM.
///
/// Until the user sets the time, the uptime stays fixed at whatever value
/// was stored in EEPROM. Once the user sets the time, the value starts
/// updating to EEPROM every 15 minutes. So in the case of intermittent
/// power, this will tell you the duration of the first interval. Which is
/// not a bad thing to know.
void updateUptime();

/// Has the user set the time?
///\note The uptime is only updated when the time is set. This gives
/// the user the opportunity to see the uptime before the first power
/// failure if power is intermittent.
bool isTimeSet();

void setUptime(uint32_t u, bool initializing = false);

uint32_t getUptime();

/// Set the low-voltage alarm threshold.
void setLowVoltageThreshold(int32_t v);

/// Get the low-voltage alarm threshold.
int32_t getLowVoltageThreshold();

/// Set the high-voltage alarm threshold.
void setHighVoltageThreshold(int32_t v);

/// Get the high-voltage alarm threshold.
int32_t getHighVoltageThreshold();

/// Get the default dit duration in microseconds.
uint32_t getDitMicrosDefault();

/// Set the dit duration in microseconds.
void setDitMicros(uint32_t ms);

/// Get the dit duration in microseconds.
uint32_t getDitMicros();

/// Set the periodic announcement format.
void setAnnouncementFormat(const char* format);

/// Get the periodic announcement format.
const char* getAnnouncementFormat();

/// Set the normal announcement interval in seconds.
void setAnnouncementInterval(uint32_t seconds);

/// Get the normal announcement interval in seconds.
uint32_t getAnnouncementInterval();

/// Set the format of periodic low-voltage alarm messages.
void setAlarmLowFormat(const char* format);

/// Set the format of periodic high-voltage alarm messages.
void setAlarmHighFormat(const char* format);

/// Get the format of periodic low-voltage alarm messages.
const char* getAlarmLowFormat();

/// Get the format of periodic high-voltage alarm messages.
const char* getAlarmHighFormat();

/// Get the repetition inteval in seconds of when in the alarm state.
void setAlarmInterval(uint32_t seconds);

/// Set the repetition inteval in seconds of when in the alarm state.
uint32_t getAlarmInterval();

/// Get a message format from the serial console.
bool readMessageFormat(char* buffer, size_t bufferSize, const char* prompt);
