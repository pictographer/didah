didah
=====

Morse code by touch and tones for a personal digital assistant

Didah is a C++/Arduino application for a Teensy 3.0 microprocessor development board. The minimal hardware setup is Teensy board wired up to a pair of headphones.

I have several goals for the project: 
- Learn about the ARM Cortex-M4 using the Freescale Kinetis MK20DX128
- Develop a reusable base for home and garden automation projects
- See how useful a computer with two buttons for input and audio for output can be
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
