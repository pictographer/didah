// -*- mode: C++; -*-
/**\file mac.h
\brief Platform-specific code to retrieve the MAC address

From THT and cmason. See 
http://forum.pjrc.com/threads/91-teensy-3-MAC-address
*/

extern uint8_t mac[6];

/// Read the MAC address into an internal buffer.
void read_mac();


/// Print the MAC address to the serial port.
void print_mac();

