// -*- mode: C++; -*-
/**@file*/
///\file didah.ino
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
///    * a terminal block transmit output (TX).
/// The terminal block also has a transmit enable (TE) open collector
/// output (push to talk).
///
/// The voltage monitor has a green LED and a red LED.
///
/// The touch sensor buttons function as an Iambic keyer with one
/// button for dit, and the other for dah. The serial port provides a
/// command line interface for viewing and modifying settings.
///
/// Setting can also be updated using a Morse code interface. Symbols
/// are pushed on a stack. When the contents of the stack match a
/// command, the command is executed. Like an HP calculator, a typical
/// command sequence has arguments entered, followed by a command.
///
/// ![Didah Voltage Monitor schematic](schematic/didah_schematic.png)

///\page 1 License License Agreement
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

/** \page 2 Instructions
# The Didah Voltage Monitor Appliance

The Didah Voltage Monitor Appliance is intended to periodically report
battery voltage for an autonomous amatuer radio repeater station. It
transmits a Morse code message on the TX output at a 3.3V signal
level. The appliance is intended to monitor and draw power from a 12V
rechargable battery.

The CPU module in Didah is a Teensy3 running at 24 MHz. There is a
voltage divider and voltage regulator designed support voltages up to
20V. The lowest voltage that can be monitored is approximately
2.7V.

## Setting Operational Parameters via Morse Code

Two touch sensor rivets on the case labeled \c (.) and \c (-) operate
as an Iambic keyer. Input is entered in International Morse Code. The
dit duration is initially 100 milliseconds, although this can be
adjusted.

Audio output is generated three ways:
- An internal speaker
- A stereo headphone jack wired to the right channel only intended
for demonstrations or noisy environments
- The TX output terminal to be wired to the repeat transmitter
- The TE output is an open collector that activates 120 ms prior to
transmission.

A very abbreviated menu can be requested by entering an M
(`--`). Output can be aborted by holding a key for approximately 100
milliseconds, but this action may be interpreted as a command.

Commands are keyed in three ways:
- Simple commands consist of a single Morse code token, such as M to
request the menu of commands.
- Commands that require arguments are entered by first entering the
arguments and then entering the command. This makes it possible to
review and correct the arguments before committing to the action.
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
      A | Set low-voltage alarm threshold in millivolts [2700-16000]
      B | Set high-voltage alarm threshold in millivolts [2700-16000]
      D | Set dit duration in milliseconds [20-200]
      F | Set normal announcement format
      G | Set low-voltage announcement format
      H | Set high-voltage announcement format
      I | Set normal announcement interval in seconds [20-86400]
      J | Set alarm announcement interval in seconds [20-86400]
      L | Print settings and log
      O | Load settings from EEPROM
      P | Set output frequency (pitch) in Hz [55-1760]
      S | Save settings to EEPROM
      T | Set the 24-hour time [0000-2359]

### The Help Command

The Help command (`?`) prints a banner, settings, and a brief list of commands.

### Setting the Low-voltage Alarm Threshold

The Low-voltage Alarm Threshold command (`A`) prompts for a value in
millivolts, ranging from 2700 to 16000 millivolts. When the input voltage
falls below this threshold, alternate values are used for the announcement
repetition format and interval. The red LED is lit continuously at a dim level.
The default threshold is 2700 millivolts.
\htmlonly
<p><code>
&gt; <b>A</b></br>
Enter low-voltage threshold in mV [2700-16000]: <b>4000</b></br>
Low-voltage threshold: 4000 mV</br>
&gt;
</code></p>
\endhtmlonly

### Setting the Low-voltage Alarm Threshold

The High-voltage Alarm Threshold command (`B`) prompts for a value in
millivolts, ranging from 2700 to 16000 millivolts. When the input voltage
rises above this threshold, alternate values are used for the announcement
repetition format and interval. The red LED is lit continuously at a dim level.
The default threshold is 16000 millivolts.

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

### Setting the Low-Voltage Alarm Announcement Format

The Low-Voltage Announcement Format command (`G`) prompts the user for
a text template for periodic announcements when the voltage is below
the nominal range. The template format is identical to the nominal
announcement format above. The default alarm format is `BAT 4801 MV ?
4801 MV`.

### Setting the High-Voltage Alarm Announcement Format

The High-Voltage Alarm Format command (`H`) prompts the user for a
text template for periodic announcements when the voltage is above the
nominal ramge. The template format is identical to the nominal
announcement format above. The default alarm format is `BAT 16000 MV ?
16000 MV`.

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
Version: 1.3
Documentation: http://pictographer.com/didah

Device to periodically announce input voltage in Morse code.

Measurements
--------
Time: 1514
Uptime: 315 minutes
Measured voltage: 4939 mV (Nominal)
Temperature: 25.87 C

Settings
--------
Nominal announcement format: BAT %V MV ? %V MV
Nominal announcement interval: 600 seconds
Low-voltage alarm threshold: 2700 mV
Low-voltage alarm announcement format: LOW V %V ? %V TEMP %C C
High-voltage alarm threshold: 16000 mV
High-voltage alarm announcement format: HIGH V %V ? %V TEMP %C C
Alarm interval: 60 seconds
Dit duration: 40 ms
Output frequency: 900 Hz
~~~~

### Loading Persistent Settings

The Load Settings command (O) restores settings saved previously.  \b
N.B. This command takes effect immediately without confirmation. Any
unsaved changes are overwritten.

In the event of a temporary power failure, settings revert to their
default values; the settings saved by the user are not automatically
restored.

### Setting the Output Frequency (Side Tone)

The output frequency or side tone can be set using the (P) command. The
frequency range is 55 to 1760 Hz. The default frequency is 750 Hz.

### Saving Settings

The Save command (S) stores the current settings to EEPROM. These settings
are preserved when there is no power to the appliance. \b N.B. This
command takes effect immediately without confirmation. Any unsaved
changes are overwritten.

### Setting the Time

The Time command (T) prompts the user for the current 24-hour time
[0000-2359].  The time in seconds is used to determine the timing of
the announcements. \b N.B. The uptime stored in EEPROM does not get
updated unless the time is set. This allows the user to see how long
the appliance was up before a power failure.

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
B  HIGH V IN MV,
C  CLEAR,
D  DIT MS,
F  MSG FMT,
G  ALARM LOW FMT,
H  ALARM HIGH FMT,
I  RPT SECS,
J  RPT ALARM SECS,
L  LIST,
M  MENU,
O  LOAD,
P  TONE HZ,
S  SAVE,
T  TIME,
U  UPTIME.
K~~~~

### Setting the Low-voltage Alarm Threshold

The `.-` (A) command expects a number ranging from 2700 to 16000 to
have been entered and sets the low-voltage alarm threshold
accordingly.

For example, to set the low-voltage alarm threshold to 5V,

User: `...../-----/-----/-----/.-` \n
System: `-.-` \n

If the number is out of range, the system will transmit 8 dits
followed by the word "RANGE".

### Setting the High-voltage Alarm Threshold

The `-...` (B) command expects a number ranging from 2700 to 16000 to
have been entered and sets the high-voltage alarm threshold
accordingly.

For example, to set the high-voltage alarm threshold to 14V,

User: `.----/....-/-----/-----/-----/-...` \n
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

### Setting the Alarm Low-Voltage Announcement Format

The `--.` (G) command causes the system to prompt for a low-voltage
alert format string.  Refer to the F command above for details.

### Setting the Alarm High-Voltage Announcement Format

The `....` (H) command causes the system to prompt for a high-voltage
alert format string. Refer to the F command above for details.

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
MSG RPT SKED IS 600 SECONDS.
MSG FORMAT IS  BAT ?V MV ? ?V MV .
ALARM VOLTAGE IS 4000  MV.
ALARM MSG RPT SKED IS 30 SECONDS.
ALARM MSG FORMAT IS  BAT ?V MV ? ?V MV .~~~~

### Loading Persistent Settings

The `---` (O) command loads settings from EEPROM. Saved settings are not
automatically restored at power on.

### Setting the Output Frequency (Side Tone)

The `.--.` (P) command sets the output frequency or side tone. The
frequency range is 55 to 1760 Hz. The default frequency is 750.

### Saving Settings

The `...` (S) command saves settings to EEPROM. Settings are retained
when there is no power. Settings are not automatically loaded when
power is restored, thus providing a fail-safe mechanism for recovering
from mis-configuration.

### Setting the Time

The `-` (T) command expects the 24-hour time to have been entered as
four digits HHMM (leading zeros may be omitted) and sets the time
accordingly. The seconds are implicitly set to zero. Until the time
is set, the uptime stored in EEPROM is not updated.

### Stack Manipulation Commands

The `-.-.` (C) command clears the stack.

The `......` (<span class="prosign">SS</span>) command pops the stack
(erases the last element).

The `-..-` (X) command exchanges the top two entries of the stack.

*/

/** \page 3 Didah Personal Digital Assistant
![PDA Exterior](../../pics/pda_exterior.jpg)
![PDA Interior](../../pics/pda_interior.jpg)
![PDA Components](../../pics/pda_components.jpg)
 */

/** \page 4 Didah Voltage Monitor Appliance
![Voltage Monitor in action](../../pics/voltage_monitor_action.jpg)
![Voltage Monitor ports](../../pics/voltage_monitor_ports.jpg)
![Voltage Monitor interior](../../pics/voltage_monitor_interior.jpg)
![Voltage Monitor bottom](../../pics/voltage_monitor_bottom.jpg)
 */

/** \page 5 Software Development
The code is hosted at https://github.com/pictographer/didah
and compiled with Teensyduino, Version 1.16 under OS X 10.8.4.

Didah started as an experiment in bootstrapping. How effectively could
one develop and debug using touch sensors as input and beeps as
output, with the aim of supporting home and garden automation? The
idea evolved into a portable gadget designed for learning Morse code,
with the aim of becoming a Personal Digital Assistant (PDA), perhaps
of interest to people who could not easily use a screen. A friend
asked if it could be made to monitor a voltage that transmits readings
via Morse code. This seemed like a good opportunity to do something
useful, so here it is. The PDA code is still present, but not
connected to the user interfaces. The previous aims have not been
forgotten. Most of the development would be useful for multiple use
cases. For example, garden applications could take advantage of the
ADC battery monitoring and could communicate by flashing lights in
Morse code.

The required hardware is a Teensy3, $19 at the time of writing.
http://pjrc.com/store/teensy3.html

For the Morse Code voltage monitor, a voltage regulator and voltage
divider are also needed. The USB power input to the Teensy3 has been
cut so that there is no conflict between the battery input and the USB
serial connection. Other power arrangements are described on the
Forum. I intend to make photographs, a bill of materials, and
schematic available.

The PJRC discussion forum is http://forum.pjrc.com/forum.php and my
user name there is pictographer.

Paul Stoffrengen provides support above and beyond anything I've seen
elsewhere for his products. He inspires the community around him to
collaborate and contribute.

 */

/** \page 6 Bill of Materials

The voltage monitor was assembled from the following components:

- 1 Teensy3
http://www.pjrc.com/store/teensy3.html
- 1 stereo minijack
- 2 small rivets
- 1 voltage regulator LP2950CZ-3.3 TO-92
http://www.onsemi.com/pub_link/Collateral/LP2950-D.PDF
- 2 tantalum capacitors 10uF 6V
- 1 1 MOhm 5% resistor
- 1 120 KOhm 5% resistor
- 2 Ceramic capacitors 222M 2.2nF
- 1 small speaker (specifications not known)
- 1 8x1 female header at 0.1" spacing
- 1 red SMT LED
- 1 green SMT LED
- 5 330 Ohm SMT resistors
- 2 2x1 terminal blocks
- 1 thin blank fiberglass board
- 1 small protoboard approximately 6 x 9 holes at 0.1"
- Assorted wire including 22 gauge solid tinned wire for connecting to
the female header, and higher gages for point-to-point wiring
- 1 plastic box LMB #P100
width 1.51", length 2.26", height 0.785"
http://www.lmbheeger.com/products.asp?catid=59
Note: this project box has two screw pillers that prevent the T3 from
being mounted length-wise. The T3 is just slightly longer than the box
is wide on the interior, so it doesn't really fit the other way either
without some very precise carving.
- Epoxy, super glue, silicon adhesive
- 1 fine-tipped permanent marker

The PDA was assembled from the following components:

- 1 Teensy3
http://www.pjrc.com/store/teensy3.html
- 1 Altoids Smalls container
- 1 stereo minijack
- 1 thin blank fiberglass board
- 1 10 KOhm trim potentiometer
- 1 2-pin JST battery connector
- 1 850 mAh lithium ion battery
https://www.sparkfun.com/products/341
- 2 chrome hole covers
- heatshrink tubing for insulation between the hole covers and the case
- 1 usb battery charger
http://www.adafruit.com/products/1304?gclid=CMXfh_jLzrkCFYs1QgodM3AAyw
- Assorted wire

Tools:

- Rotary tool
- Razor knife
- Small files
- Soldering iron and related tools
- Small vice

Caveats: I'm a novice at electronics. It's very unlikely I picked the
best components, the best way of assembling them, or the best circuit
for this purpose. That said, everything described here works.
 */

#include <cctype>
#include "Arduino.h"
#include <EEPROM.h>
#include "EEPROMAnything.h"
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
   assignAction(a2m('O'), dispatchOnStack);
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
   assignAction(MorseToken(MORSE_SS), dispatchOnStack);

#if defined(PDA)
   assignAction(MorseToken(MORSE_CT), keyboardMode);
#endif
   assignAction(MorseToken(MORSE_INTER), dispatchOnStack);

   if (Serial) {
      printBanner();
   }

   uint32_t u;
   (void) EEPROM_readAnything(0, u);
   setUptime(u, true);

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
   time_t currentSeconds(now());

   // Update the voltage alarm state.
   long vin(readMillivolts());
   if (vin <= getLowVoltageThreshold()) {
      setAlarmState('v');
   } else if (getHighVoltageThreshold() <= vin) {
      setAlarmState('^');
   } else {
      setAlarmState('-');
   }

   updateUptime();

   if (token.valid()) {
      Serial.println(token.toChar());

      doAction(token);

   } else if (currentSeconds % getIntervalForState() == 0) {
      digitalWrite(txEnablePin, HIGH);
      delayMicroseconds(getRadioStartupDelay());
      txString(getFormatForState());
      digitalWrite(txEnablePin, LOW);
   } else if (Serial.available()) {
      readEvalPrint();
   } else {
      toggleGreenLED();
      redLED(getAlarmState() != '-');
   }
}
