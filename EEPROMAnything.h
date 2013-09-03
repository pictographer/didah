// -*- mode: C++; -*-
/**\file EEPROMAnything.h 
\brief Loads and stores arbitrary structures to non-volatile memory

From
http://playground.arduino.cc//Code/EEPROMWriteAnything?action=sourceblock&num=1
*/
#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

/// Dump a raw value starting at EEPROM (or Flash) location ee.
template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
	  EEPROM.write(ee++, *p++);
    return i;
}

/// Read a raw value from EEPROM (or Flash) location ee.
template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
	  *p++ = EEPROM.read(ee++);
    return i;
}
