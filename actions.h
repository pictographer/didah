// -*- mode: C++; -*-
/**\file actions.h
\brief Application-specific actions
 */
#pragma once
#include "Arduino.h"
#include "libmorse.h"
#include "morse_token.h"

const char* getVersion();

///\name Action Mapping and Dispatching
//@{

typedef void (*ActionFn)(MorseToken);

///\brief Initialize the table of function pointers to no-ops.
///
/// The actions table is indexed by the bottom 8 bits of our internal
/// representation of a Morse code character or prosign. Actions are
/// passed MorseToken instances so that a single action can handle
/// several different tokens.
///
/// For more complicated command syntax, dispatchOnStack() in
/// actions.cpp performs pattern matching on the stack and dispatches
/// to the first command that matches.
///
/// The intent was to support command rebinding at runtime while being
/// efficient enough to run on a low-powered processor.
void initActions();

/// Our internal representation of Morse code is left justified, that
/// is to say, the high order bit is the first dit/dah. Most of the
/// time, the bottom 8 bits will be zero, so create a hash by grabbing
/// the top 8 bits.
///
/// Actually, the comment above applies when MorseToken is defined to
/// be larger than 8 bits. Currently sizeof(MorseToken) == 1, so
/// codeHash(x) == x.
uint8_t codeHash(MorseToken code);

/// Hash a Morse code symbol to a table location and store action there.
void assignAction(MorseToken code, ActionFn action);

/// Invoke an action from the actions table.
void doAction(MorseToken code);

/// Save all the action pointers in preparation for overwriting some or
/// all of them with action pointers for a mode.
/// If the actions have been saved previously, the actions are not
/// saved and the previously saved actions are not disturbed.
void saveActions();

/// If there are actions saved, restore them. Otherwise, transmit
/// an error.
void restoreActions();

/// Push the most recent symbol onto the stack, and then restore
/// all the actions to their previous values.
void push1RestoreAll(MorseToken code);

///\brief Score the given pattern against the state of the symbol stack.
/**
The maximum score is 255 and decreases with increased distance
from the pattern.

\li    \c A-Z, \c 0-9, punctuation - Match a literal character
\li    \c ^CQ^ - Match a prosign
\li    space - Match a word-length or greater pause
\li    \c a - Match a single letter from A-Z
\li    \c c - Match any character
\li    \c d - Match a single digit
\li    \c n - Match a contiguous sequence of digits

With the exception of \c n, all the pattern elements match one
Morse token. The following punctuation is accepted:
\verbatim
!"$&'()+,-./:;=?@_               \endverbatim

The pattern is compared to the stack starting at the end of the
pattern and the top of the stack. There is no support for
backtracking. All pattern elements are greedy.
*/
uint8_t matchScore(const char* pattern);

/// Execute the command that best matches the stack contents.
///\note The caller graph is incorrect. A pointer to this function is
///passed in `setup()` but the caller is `doAction()`.
void dispatchOnStack(MorseToken);

/// Perform in-place conversion of the given buffer to upper case.
void mapToUpper(char* buffer, size_t n);

///\brief Check for unsupported punctuation and some invalid escape
///sequences.
///
/// Substitution placeholders %C, %F, %T, %U, and %V are checked;
/// prosign escape sequences are not.
bool isValidMorseFormat(const char* buffer, size_t n);

/// Respond to commands from the serial port.
void readEvalPrint();

//@}

///\name Operations
//@{

/// Transmit the top element of the stack.
void txSymbolStackTop(MorseToken);

/// List the contents of the symbol stack.
void txListSymbolStack(MorseToken);

/// Transmit the 24-hr time.
void txTimeHHMM(MorseToken code);

/// Transmit the time verbosely. The announcement starts with QRT and
/// repeats the time twice.
void txTimeVerbose(MorseToken code);

/// Transmit the uptime.
void txUptime(MorseToken);

/// Transmit the internal temperature as an integer in Celsius.
void txInternalTemperatureC(MorseToken code);

/// Transmit the internal temperature in integer in Fahrenheit.
void txInternalTemperatureF(MorseToken code);

/// Transmit the mac address.
void txMacAddress(MorseToken code);

/// Increase the unit time by 5000 microseconds.
void slowDown();

/// Decrease the unit time by 5000 microseconds.
void speedUp();

/// Transmit the \em AR prosign three times.
void txAR(MorseToken);

/// Transmit II \<code\> \em AR
void txEcho(MorseToken code);

/// Transmit CQ CQ CQ DE ET1 ET1 K
void txGreeting(MorseToken code);

/// Play a tone with pitch proportional to the pressure on the dit
/// button.
void getPressure(MorseToken code);

/// Report the current settings and sensor values via the serial port.
void printLog();

/// Print documentation for the commands via the serial port.
void printBanner();

///\brief Attempt to control the output volume but adjusting the PWM duty
/// cycle.
///
///\note This approach is not terribly effective. Perhaps for lack of
/// proper output filtering.
void setVolume(MorseToken);

/// Transmit help text.
void txHelp(MorseToken);

/// Transmit a tersely-worded command list.
void txMenu(MorseToken);

/// Transmit the current settings and sensor values.
void txSettings(MorseToken);

//@}

///\name Aural Feedback
//@{

/// Play a brief high-pitched chirp to signify the end of the waiting
/// period for adding symbols to a token.
void txTick(MorseToken code);

/// Play a brief pair high-pitched chirps.
void txTiTick(MorseToken code);

/// Play a brief high-pitched chirp to signify the end of the waiting
/// period for adding characters to a word.
void txTock(MorseToken code);

//@}

///\name Predicates
//@{
typedef int (*IntPredicate)(int);
//@}

///\name Morse Code Input
//@{

///\brief Prompt for a value from the serial port.
///
/// If the value read is within the given range, assign it to the
/// result. A true return value indicates a value within range was
/// obtained.
bool readRange(uint32_t& result,
	       const char* prompt,
	       uint32_t minimum,
	       uint32_t maximum);

///\brief Read a line from the serial port with support for rubbing
/// out preceding characters.
void readEdit(char* buffer, bool& isNotTooLate,
              const char* end, IntPredicate isIncluded, uint32_t timeout);

///\brief Read a string from the serial terminal up to a return or the
/// end of the buffer. Return false if timeout milliseconds elapse
/// before either a return is sent or the buffer is filled including
/// null termination. In all cases the buffer is null-terminated.
bool readLine(char* buffer, size_t n, uint32_t timeout=60000000);

/// Read a non-negative integer from the Serial port.
bool readNumber(uint32_t& v, uint32_t timeout=60000000);

//@}

///\brief Get the low-voltage alarm state. This determines the message
/// format and repetition interval.  Returns 'V' for low, '^' for
/// high, and '-' for nominal.
char getAlarmState();

///\brief Set the low-voltage alarm state. This determines the message
/// format and repetition interval.
void setAlarmState(char state);

///\brief A sample ADC reading is taken each time through the main
/// loop, and written to a ring buffer. The samples are only filtered
/// and converted to a voltage once every poll interval.
uint32_t getVoltagePollInterval();

///\brief Get the announcement interval for the current alarm state.
uint32_t getIntervalForState();

///\brief Get the alarm format for the current alarm state.
const char* getFormatForState();

///\brief Get the raw analog input.
void txRaw(MorseToken);
