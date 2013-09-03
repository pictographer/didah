// -*- mode: C++; -*-
///\file Morse.ino
///\brief Top-level entry points for the Morse code PDA/Application

///\mainpage
///\brief Didah - A Morse code PDA/Appliance for the Teensy 3.0 ARM Cortex-M4
/// development board \n
/// Copyright © 2013 The Pictographer's Shop. \n
///\n
///\details The hardware supplies two touch sensor buttons, an
/// audio output, and a USB serial port. For the voltage monitor
/// appliance, there are three 
/// sources of audio output:
///    * an internal speaker, 
///    * an earphone jack, and
///    * a terminal block output. 
/// The voltage monitor also has a green LED and a red LED.
///
/// The touch sensor buttons function as an Iambic Keyer with one
/// button for dit, and the other for dah. The serial port provides a
/// command line interface for viewing and modifying settings.
///
/// Setting can also be updated using a Morse code interface. Symbols
/// are pushed on a stack. When the contents of the stack match a
/// command, the command is executed. Like an HP calculator, a typical
/// command sequence has arguments entered, followed by a command.

///\page License License Agreement
/// Permission is hereby granted, free of charge, to any person obtaining
/// a copy of this software and associated documentation files (the
/// "Software"), to deal in the Software without restriction, including
/// without limitation the rights to use, copy, modify, merge, publish,
/// distribute, sublicense, and/or sell copies of the Software, and to
/// permit persons to whom the Software is furnished to do so, subject to
/// the following conditions:
/// 
/// The above copyright notice and this permission notice shall be
/// included in all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
/// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
/// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
/// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
/// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

/** \page Instructions
# The Didah Voltage Monitor Appliance

The Didah Voltage Monitor Appliance is intended to periodically report
battery voltage for an autonomous amatuer radio repeater station. It
transmits a Morse code message on the TX output at a 3.3V signal
level. The appliance is intended to monitor and draw power from a 12V
rechargable battery.

The CPU module in Didah is a Teensy3 running at 24 MHz. There is a
voltage divider and voltage regulator designed support voltages up to
20V, however voltages greater than 11.7V are not distinguished due to
a defect. The lowest voltage that can be monitored is approximately
2.7V.

## Setting Operational Parameters via Morse Code

Two touch sensor rivets on the case labeled \c (.) and \c (-) operate
as an Iambic keyer. Input is entered in International Morse Code. The
dit duration is initially 30 milliseconds, although this can be
adjusted.

Audio output is generated three ways:
- An internal speaker
- A stereo headphone jack wired to the right channel only useful
for demonstrations or noisy environments
- The TX output terminal to be wired to the repeat transmitter

A very abbreviated menu can be requested by entering an M (`--`).

Commands are keyed in three ways:
- Simple commands consist of a single Morse code token, such as M to
request the menu of commands.
- Commands that require arguments are entered after the arguments so
that it is possible to review and correct the arguments before
entering the command.
- Commands that require freeform text initiate a text copying mode
terminated by the <span class="prosign">AR</span> prosign or by a
timeout.

## Setting Operational Parameters via Serial Console

The USB Micro-B receptacle can be used to connect the appliance to a
computer. The appliance appears as a serial device (like a modem or
remote terminal). A terminal emulator such as Putty or the Linux
commands `cu` or `screen` in a command shell can be used for
command-line interface.

The serial parameters are:

Setting      | Value
-------------|------
Speed        | 9600 bps
Parity       | None
Data         | 8 bits
Stop bits    | None
Flow control | None

For Macintosh OS X, in a terminal window the following command can be used
to initiate a command-line dialog:

`sudo cu -l /dev/tty.usbmodem* -s 9600 --nostop --parity=none`

As of OS X 10.8.4, the cu program does not successfully restore the
serial connection when waking from sleep mode. It may be necessary to
kill the cu session. A cu session can usually be terminated by typing

`~.<enter>`

twice. Note that `<enter>` represents the Enter or Return key.

To use screen instead of cu:

`sudo screen /dev/tty.usbmodem* 9600`

For Linux, in a terminal window the following command can be used
to initiate a command-line dialog:

`sudo cu -l /dev/tty.usbserial-* -s 9600`

The name of the serial device may vary from session to session. If
more than one serial device is connected, the wildcard would need to
be changed to something more specific. A little quality time with the
for cu or screen might yield a way to configure Didah by inserting a
file in the serial stream or log serial output to a file.

## Settings

In the default configuration, input voltage is transmitted every 10
minutes in Morse Code as follows:

`BAT 4801 MV ? 4801 MV`

The message content, repetition interval, transmission rate, pitch
(side tone), and alarm level can all be configured. The configuration can be 
saved to persistent storage.

## Command-line Interaction

Command | Description
--------|------------
      ? | Help
      A | Set low-voltage alarm threshold in millivolts [2700-12000]
      D | Set dit duration in milliseconds [20-200]
      F | Set normal announcement format
      G | Set low-voltage announcement format
      I | Set normal announcement interval in seconds [20-86400]
      J | Set low-voltage announcement interval in seconds [20-86400]
      L | Print settings and log
      O | Load settings from EEPROM
      P | Set output frequency (pitch) in Hz [55-1670]
      S | Save settings to EEPROM
      T | Set the 24-hour time [0000-2359]

### The Help Command

The Help command (`?`) prints a banner, settings, and a brief list of commands.

### Setting the Low-voltage Alarm Threshold

The Low-voltage Alarm Threshold command (`A`) prompts for a value in
millivolts, ranging from 2700 to 12000 millivolts. When the input voltage 
falls below this threshold, alternate values are used for the announcement
repetition format and interval. The red LED is lit continuously at a dim level.
The default threshold is 2700 millivolts.
\htmlonly
<p><code>
&gt; <b>A</b></br>
Enter low-voltage threshold in mV [2700-12000]: <b>4000</b></br>
Low-voltage threshold: 4000 mV</br>
&gt;
</code></p>
\endhtmlonly

### Setting the Dit Duration

The Dit Duration command (`D`) prompts for a value in milliseconds,
ranging from 20 to 200. This sets the rate at which Morse Code is
entered and transmitted. The default duration is 40 milliseconds.

### Setting the Normal Announcement Format

The Normal Announcement Format command (`F`) prompts the user for a text
template for periodic announcements when the voltage is at a nominal
level.

The format template for announcements uses the following notation:

Symbol         | Interpretation
---------------|---------------
A-Z            | Literal character
0-9            | Literal number
\e space       | Pause for one dit unit
\e punctuation | Literal punctuation: !"$&'()+,-./:;=?@_
\e ^ps^        | Prosign of up to seven dits and dahs. \n For example prosign BK is transmitted \c -....-.
\%V            | Monitor voltage in millivolts
\%F            | Temperature in degrees Fahrenheit
\%C            | Temperature in degrees Celsius
\%T            | Time of day in four-digit 24-hour time: HHMM
\%U            | Uptime in four digits: HHMM

The default announcement format is `BAT 4801 MV ? 4801 MV`.

### Setting the Alarm Announcement Format

The Normal Announcement Format command (`G`) prompts the user for a
text template for periodic announcements when the voltage is below the
nominal level. The template format is identical to the nominal
announcement format above. The default alarm format is `BAT 4801 MV ?
4801 MV`.

### Setting the Normal Announcement Interval

The Normal Announcement Interval command (`I`) prompts the user for a
time interval in seconds ranging from 20 to 86,400 (24 hours). When
the voltage is nominal, and the time in seconds is a multiple of this
value, a nominal announcement is transmitted. The default announcement 
interval is 600 seconds (ten minutes).

### Setting the Alarm Announcement Interval

The Alarm Announcement Interval command (`J`) prompts the user for a
time interval in seconds ranging from 20 to 86,400 (24 hours). When
the voltage is at or below the alarm threshold, and the time in
seconds is a multiple of this value, an alarm announcement is
transmitted. The default alarm interval is 30 seconds.

### The Log Command

The Log command (`L`) causes the current readings and settings to be printed.

A typical log message is as follows:
~~~~
Didah Voltage Monitor
=====================
Version: 1.1
Documentation: http://pictographer.com/didah

Device to periodically announce input voltage in Morse code.

Measurements
--------
Time: 1736
Uptime: 0 minutes
Measured voltage: 4858 mV
Temperature: 27.93 C

Settings
--------
Nominal announcement format: BAT %V MV ? %V MV
Nominal announcement interval: 600 seconds
Low-voltage alarm threshold: 2700 mV
Low-voltage alarm announcement format: BAT %V MV ? %V MV
Alarm interval: 30 seconds
Dit duration: 40 ms
Output frequency: 750 Hz ~~~~

### Loading Persistent Settings

The Load Settings command (O) restores settings saved previously.
\b N.B. This command takes effect immediately without confirmation. Any unsaved
changes are overwritten.

\e \b Question: 
Should the device power on with factory settings, thus 
providing a fail-safe mode for user configuration errors; or should the device
restore the user's saved settings so that autonomous restarts only lose the
time-of-day.

### Setting the Output Frequency (Side Tone)

The output frequency or side tone can be set using the (P) command. The
frequency range is 55 to 1670 Hz. The default frequency is 750.

### Saving Settings

The Save command (S) stores the current settings to EEPROM. These settings
are preserved when there is no power to the appliance. \b N.B. This 
command takes effect immediately without confirmation. Any unsaved
changes are overwritten.

### Setting the Time

The Time command (T) prompts the user for the current 24-hour time [0000-2359].
The time in seconds is used to determine the timing of the announcements.

## Morse Code Interaction

The Morse Code interface is designed to provide a quick method for an
experienced CW operator to adjust the settings of the voltage monitor
appliance. As mentioned above, commands are keyed in three ways:
- Simple commands consist of a single Morse code token, such as M to
request the menu of commands.
- Commands that require arguments are entered after the arguments so
that it is possible to review and correct the arguments before
entering the command.
- Commands that require freeform text initiate a text copying mode
terminated by the <span class="prosign">AR</span> prosign or by a
timeout.

### The Menu Command

The Menu Command (`--`) transmits an abbreviated list of commands:
~~~~
A  LOW V IN MV,
C  CLEAR, 
D  DIT MS, 
F  MSG FMT, 
G  ALARM FMT, 
H  ERASE ONE, 
I  RPT SECS, 
J  RPT ALARM SECS, 
L  LIST, 
P  TONE HZ, 
T  TIME, 
U  UPTIME. 
K~~~~

### Setting the Low-voltage Alarm Threshold

The `.-` (A) command expects a number ranging from 2700 to 12000 to
have been entered and sets the low-voltage alarm threshold
accordingly.

For example, to set the low-voltage alarm threshold to 5V, 

User: `...../-----/-----/-----/.-` \n
System: `-.-` \n

If the number is out of range, the system will transmit 8 dits 
followed by the word "RANGE".

### Setting the Dit Duration

The `-..` (D) command expects a number of milliseconds ranging from 20
to 200 to have been entered and sets the dit duration accordingly.

For example, to set the dit duration to 50ms,

User: `...../-----/.-` \n
System: `-.-` \n

If the number is out of range, the system will transmit 8 dits 
followed by the word "RANGE".

### Setting the Normal Announcement Format

The `..-.` (F) command causes the system to prompt for a format template.
The system responds with 
`TX MSG END WITH` <span class="prosign">AR</span>  `K`.

For example, here is how to make the system transmit the voltage
followed by MV:

User: `.._.` \n
System: `_ _.._ /__ ... __. /. _. _.. /.__ .. _ .... /._._. //_._` \n
User: `..--../...-//--/...-/.-.-.` \n
System: `-.-` \n

Or in text,

User: F\n
System: TX MSG END WITH <span class="prosign">AR</span> OVER\n
User: ?V MV <span class="prosign">AR</span>\n
System: OVER\n

\b N.B. To indicate a placeholder, key a question mark (`..--..`)
followed immediately by one of V, F, C, T, or U. A question mark
followed by a pause or any other token is transmitted literally.

The format template is used for generating periodic messages when the 
voltage level is nominal just prior to message generation. The default 
format is `BAT 4801 MV ? 4801 MV`.

### Setting the Alarm Announcement Format

The `--.` (G) command causes the system to prompt for a format string. 
Refer to the F command above for details.

### Setting the Normal Announcement Interval

The `..` (I) command expects a number of seconds ranging from 20
to 86400 to have been entered and sets the nominal announcement interval 
accordingly. The interval is coordinated with the time of day. The default
interval is 600 seconds (ten minutes).

### Setting the Alarm Announcement Interval

The `.---` (J) command expects a number of seconds ranging from 20
to 86400 to have been entered and sets the alarm announcement interval 
accordingly. The interval is coordinated with the time of day. The default 
alarm interval is 30 seconds.

### The Log Command

The `.-..` (L) command transmits a log of the current settings. 
For example,

~~~~
SETTINGS.
MSG RPT SKED IS  6_____ 0_____ 0 SECONDS.
MSG FORMAT IS  BAT ?V MV ? ?V MV . 
ALARM VOLTAGE IS 4_____ 0_____ 0_____ 0  MV. 
ALARM MSG RPT SKED IS 30 SECONDS. 
ALARM MSG FORMAT IS  BAT ?V MV ? ?V MV .~~~~

### Loading Persistent Settings

The `---` (O) command loads settings from EEPROM.

### Setting the Output Frequency (Side Tone)

### Saving Settings

The `...` (S) command saves settings to EEPROM. Settings are retained when there is no power. Settings are not automatically loaded when power is restored, thus providing a fail-safe mechanism for recovering from mis-configuration.

### Setting the Time

The `-` (T) command expects the 24-hour time to have been entered as
four digits HHMM (leading zeros may be omitted) and sets the time
accordingly. The seconds are implicitly set to zero.

*/

#include <cctype>
#include "Arduino.h"
#include <EEPROM.h>
#include "settings.h"
#include "actions.h"
#include "stack.h"
#include "prosign.h"
#include "Time.h"

/// Arduino is basically C++ with a default main() that calls setup()
/// once, and then calls loop() repeatedly.
void setup() {
   initPorts();

   initActions();
   for (size_t i(0); i < 10; ++i) {
      assignAction(a2m(i + '0'), pushSymbolStack);
   }
   assignAction(a2m('A'), dispatchOnStack);
   assignAction(a2m('B'), dispatchOnStack);
   assignAction(a2m('C'), dispatchOnStack);
   assignAction(a2m('D'), dispatchOnStack);
   assignAction(a2m('F'), dispatchOnStack);
   assignAction(a2m('G'), dispatchOnStack);
   assignAction(a2m('H'), dispatchOnStack);
   assignAction(a2m('I'), dispatchOnStack);
   assignAction(a2m('J'), dispatchOnStack);
   assignAction(a2m('K'), dispatchOnStack);
   assignAction(a2m('L'), txSettings);
   assignAction(a2m('M'), txMenu);
   assignAction(a2m('P'), dispatchOnStack);
   assignAction(a2m('Q'), dispatchOnStack);
   assignAction(a2m('R'), dispatchOnStack);
   assignAction(a2m('S'), dispatchOnStack);
   assignAction(a2m('T'), dispatchOnStack);
   assignAction(a2m('U'), txUptime);
   assignAction(a2m('V'), dispatchOnStack);
   assignAction(a2m('X'), dispatchOnStack);

   assignAction(MorseToken(MORSE_AR), dispatchOnStack);
   assignAction(MorseToken(MORSE_AS), dispatchOnStack);

   assignAction(MorseToken(MORSE_CT), keyboardMode);
   assignAction(MorseToken(MORSE_INTER), dispatchOnStack);

   if (Serial) {
      printBanner();
   }
   txString("K");
   Serial.setTimeout(3 * 1000);
}

/// The main loop polls the source voltage, touch sensors, serial port
/// input buffer, and time. It dispatches commands entered via serial
/// port or touch sensors. It updates the status LEDs.
void loop() {
   sampleVoltage();
   // Gather Morse code symbols from touch input and invoke an
   // evaluation function on them.
   MorseToken token(getMorse());
   if (now() % getVoltagePollInterval() == 0) {
      setAlarmState(readMillivolts() <= getLowVoltageThreshold());
   }
   if (token.valid()) {
      Serial.println(token.toChar());

      doAction(token);

   } else if (now() % getIntervalForState() == 0) {
      txString(getFormatForState());
   } else if (Serial.available()) {
      readEvalPrint();
   } else {
      toggleGreenLED();
      if (getAlarmState()) {
         toggleRedLED();
      }
   }
}