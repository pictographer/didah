Didah
=====

Morse code by touch and tones for a personal digital assistant and amateur radio repeater voltage monitor appliance.

Didah is a C++/Arduino application for a Teensy 3.0 microprocessor development board. The minimal hardware setup is Teensy board wired up to a pair of headphones.

I have several goals for the project: 
- Learn about the ARM Cortex-M4 using the Freescale Kinetis MK20DX128
- Develop a reusable base for home and garden automation projects
- See how useful a computer with two buttons for input and audio for output can be
- Develop a complete self-contained voltage monitor appliance for the amateur radio community
- Share some code with the Teensy 3.0, Arduino, and Morse Code communities
- Offer something to anyone who might appreciate an alternative to screens and keyboards

The following features are present:
- Touch sensor auto-calibration
- Iambic-style Morse code entry via two touch sensors, one for dit and the other for dah.
- Tone generation using the pulse width modulation hardware
- Musical note generation by name
- International Morse Code prosigns and Q codes
- Time keeping and alarm
- Tutorial song ABCs as Morse code
- Simple arithmetic game for number practice
- Simple letter game for letter practice
- Pattern-based command recognizer
- Morse code USB keyboard (User taps Morse code and letters appear as if typed on a computer keyboard)
- Transmission rate and pitch control
- Analog touch sensing
- Report temperature
- Report MAC address
- Uptime tracking that persists across power failures
- Type-safety for the Morse code binary coding scheme
- Voltage monitoring via built-in ADC
- Median filtering for ADC values
- Calibrated temperature measurement
- Message format templates a la printf, but for voltage, temperature, and time

The following features are under develpment or consideration:
- Low-power mode
- Sound effects such as different timbres for different modes
- Automatic input error correction
- List making or simple note taking
- Simple programming of triggers and actions via Morse code
- A password safe
- A calculator

Prototype hardware configuration
- Teensy 3.0
- Altoids Smalls case
- 850 mAh lithium-ion battery
- Stereo minijack
- Trim potentiometer volume control
- Two chrome hole covers as buttons
- 32 kHz crystal for realtime clock

Voltage monitor hardware configuration
- Teensy 3.0
- Small plastic project case
- External battery from 2.7V to 12V
- Stereo minijack
- Screw terminals for +12V, TX, and GND
- Two touch buttons
- Red LED
- Green LED
- Piezoelectric speaker
- Voltage regulator
- Voltage divider

See http://pictographer.com/didah for voltage monitor appliance documentation.
See https://github.com/pictographer/didah for source code.