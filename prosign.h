// -*- mode: C++; -*-
///\file prosign.h
///\brief Prosign constants
///\todo Convert preprocessor defines to constant Morse tokens.
#pragma once
#include "binconst.h"
#include <climits>

/// Generate a binary literal with upto 7 bits in a 16-bit field left
/// justified and terminated with a 1 as a sentinel. To serialize the
/// bits from left to right, extract the left-most bit by masking
/// against MORSE_MSB and left shift one place.
#define M8(d) \
(B8(d##1) << (CHAR_BIT * (sizeof MorseToken(0)) - sizeof #d))

/// CRLF
#define MORSE_AA     M8(0101)

/// Same as MORSE_SN
#define MORSE_ACK    M8(00010)

/// Same as MORSE_WAIT and MORSE_AS?
#define MORSE_AMPER  M8(01000)

#define MORSE_APOSTR M8(011110)

/// Finished copying
#define MORSE_AR     M8(01010)

/// Wait, optionally followed by 1-9 for minutes.
#define MORSE_AS     MORSE_AMPER

#define MORSE_AT     M8(011010)

/// Paragraph. "Begin two lines." (AKA MORSE_DDASH)
#define MORSE_BT     M8(10001)

/// Break. "Back-to-you"
#define MORSE_BK     M8(1000101)

#define MORSE_CLOSEP M8(101101)
#define MORSE_COLON  M8(111000)
#define MORSE_COMMA  M8(110011)

/// Going off the air
#define MORSE_CL     B16(10100100,10000000)

/// Start copying. "Copy this."
#define MORSE_CT     M8(10101)

/// Clear to send. (AKA MORSE_OK)
#define MORSE_CTS    M8(101)
#define MORSE_DDASH  MORSE_BT
#define MORSE_DOLLAR M8(0001001)
#define MORSE_END    M8(000101)
#define MORSE_ERROR  B16(11111111,00000000)
#define MORSE_EXCLAI M8(101011)
#define MORSE_INTER  M8(001100)

/// Ok, named-station. Only the named station should reply.
#define MORSE_KN     M8(01101)

#define MORSE_MINUS  M8(100001)

/// End of statement/ready for reply. "Ok, go ahead."
#define MORSE_OK     MORSE_CT

#define MORSE_OPENP  M8(10110)
#define MORSE_PERIOD M8(010101)
#define MORSE_PLUS   MORSE_AR
#define MORSE_QUOTE  M8(010010)
#define MORSE_SEMI   M8(101010)
#define MORSE_START  MORSE_CT

/// End of contact. "Silent key"
#define MORSE_SK     MORSE_END

#define MORSE_SLASH  M8(10010)

/// Understood. "Sho'nuff."
#define MORSE_SN     MORSE_ACK

/// SOS. Not to be used unless there is iminent danger to life or
/// destruction of property.
#define MORSE_SOS    B16(00011100,01000000)

#define MORSE_UNDER  M8(001101)
#define MORSE_WAIT   M8(01000)
