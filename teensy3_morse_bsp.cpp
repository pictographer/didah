/// Implementation of Teensy 3.0 board support package

///\file teensy3_morse_bsp.cpp

#include <climits>
#include "teensy3_morse_bsp.h"
//#include "MK20DZ10.h"
#include "mk20dx128.h"
#include "cpu.h"
#include "libmorse.h"
#include "voltage_monitor.h"
//---- Teensy 3.0 board support ----

char cpuid[] = "012345678911234567892123456789";

char* identifyProcessor() {
  uint8_t i(0);

  // Family
  uint8_t fam((SIM_SDID & SIM_SDID_FAMID(0x7)) >> SIM_SDID_FAMID_SHIFT);
  cpuid[i++] = (fam < 8) ? 'K' : '?';
  fam = (fam < 8) ? fam : 8;
  cpuid[i++] = "1234675?"[fam];
  cpuid[i++] = "0000003?"[fam];
  
  cpuid[i++] = '-';

  uint8_t pkg(((SIM_SDID & SIM_SDID_PINID(0xF)) >> SIM_SDID_PINID_SHIFT) - 2);
  uint8_t pinTable[] = {32U, 0, 48U, 64U, 80U, 81U, 100U, 104U, 0, 144U, 196U, 0, uint8_t(256U) };
  pkg = (pkg < sizeof pinTable) ? pkg : 1;
  uint8_t pinCount(pinTable[pkg]);
  if (99 < pinCount) {
    cpuid[i++] = '0' + pinCount / 100;
  }
  if (9 < pinCount) {
    cpuid[i++] = '0' + pinCount / 10  % 10;
  }
  if (0 < pinCount) {
    cpuid[i++] = '0' + pinCount % 10;
  } else {
    cpuid[i++] = '?';
  }

  cpuid[i++] = '-';

  // Silicon revision
  cpuid[i++] = '1';
  cpuid[i++] = '.';
  cpuid[i++] = '0' + ((SIM_SDID & SIM_SDID_REVID(0xF)) >> SIM_SDID_REVID_SHIFT);

  cpuid[i++] = '-';

  // P-flash size
  uint8_t pf(((SIM_FCFG1 & SIM_FCFG1_PFSIZE(0xF)) >> SIM_FCFG1_PFSIZE_SHIFT)
	     - 7);
  uint16_t pfTable[] = { 128U, 0, 256U, 0, 512U, 0, 0, 0, 512U };
  if (pf < sizeof pfTable) {
    cpuid[i++] = '0' + pfTable[pf] / 100;
    cpuid[i++] = '0' + pfTable[pf] / 10 % 10;
    cpuid[i++] = '0' + pfTable[pf] % 10;
  } else {
    cpuid[i++] = '?';
  }

  cpuid[i++] = '-';

  // FlexNVM size
  // xxx ???
  if (SIM_FCFG2 & SIM_FCFG2_PFLSH_MASK) {
    cpuid[i++] = '0';
  } else {
    uint8_t fnvm((SIM_FCFG1 & SIM_FCFG1_NVMSIZE(0xF))>>SIM_FCFG1_NVMSIZE_SHIFT);
    uint8_t fnvmSize(1 << (6 + (fnvm >> 1)));
    cpuid[i++] = '0' + fnvmSize / 100;
    cpuid[i++] = '0' + fnvmSize / 10 % 10;
    cpuid[i++] = '0' + fnvmSize % 10;
  }

#if 0
    /* Determine the RAM size */
    switch((SIM_SOPT1 & SIM_SOPT1_RAMSIZE(0xF))>>SIM_SOPT1_RAMSIZE_SHIFT)
    {
    	case 0x5:
    		Serial.println("32 kBytes of RAM\n\n");
    		break;
    	case 0x7:
    		Serial.println("64 kBytes of RAM\n\n");
    		break;
    	case 0x8:
    		Serial.println("96 kBytes of RAM\n\n");
    		break;
    	case 0x9:
    		Serial.println("128 kBytes of RAM\n\n");
    		break;
    default:
      Serial.print("RAM ");
      Serial.println((SIM_SOPT1 & SIM_SOPT1_RAMSIZE(0xF))>>SIM_SOPT1_RAMSIZE_SHIFT);  
      break;  		
    }

	/* Determine the last cause(s) of reset */
	if (MC_SRSH & MC_SRSH_SW_MASK)
		Serial.println("Software Reset\n");
	if (MC_SRSH & MC_SRSH_LOCKUP_MASK)
		Serial.println("Core Lockup Event Reset\n");
	if (MC_SRSH & MC_SRSH_JTAG_MASK)
		Serial.println("JTAG Reset\n");
	
	if (MC_SRSL & MC_SRSL_POR_MASK)
		Serial.println("Power-on Reset\n");
	if (MC_SRSL & MC_SRSL_PIN_MASK)
		Serial.println("External Pin Reset\n");
	if (MC_SRSL & MC_SRSL_COP_MASK)
		Serial.println("Watchdog(COP) Reset\n");
	if (MC_SRSL & MC_SRSL_LOC_MASK)
		Serial.println("Loss of Clock Reset\n");
	if (MC_SRSL & MC_SRSL_LVD_MASK)
		Serial.println("Low-voltage Detect Reset\n");
	if (MC_SRSL & MC_SRSL_WAKEUP_MASK)
		Serial.println("LLWU Reset\n");	
	Serial.println(MC_SRSL); //!\todo is zero on my T3
#endif
  cpuid[i] = 0;
  return cpuid;  
}

/// PWM pins 3, 4, 5, 6, 9, 10, 20, 21, 22, 23

/// touchRead pins 0, 1, 15, 16, 17, 18, 19, 22, 23
static uint16_t touchBase[24];
uint16_t touchMargin(60);

/// Measure the noise floor for the given touch pin. A threshold is set
/// 600pF above the maximum measured value.
void calibrateTouch(uint8_t pin) {
  analogReadRes(8);
  touchBase[pin] = 0;
  for (int i(0); i < 10; ++i) {
    uint16_t pF20(touchRead(pin));
    pwmWrite(pF20);
    touchBase[pin] = touchBase[pin] < pF20 ? pF20 : touchBase[pin];
    delay(100);
  }
}

/// Return the pin capacitance in pF.
uint16_t getPinThreshold(uint8_t pin) {
  return 50 * (touchBase[pin] + touchMargin);
}

/// Poll the given capacitive sensor pin. Return true if the value
/// is greater than the calibration threshold for the pin.
bool touchPoll(uint8_t pin) {
  analogReadRes(16);
  uint16_t pF20(touchRead(pin));
  return touchBase[pin] + touchMargin < pF20;
}

size_t duty = duty50;

size_t getDuty() {
  return duty;
}

void setDuty(uint8_t dutyCycle) {
  duty = dutyCycle;
}

/// Output a square wave on pin 10 at the given frequency for the given duration.
void beep(uint32_t frequency, uint32_t durationMicros) {
  pwmFrequency(frequency);
  pwmWrite(duty);
  redLED(1);
  delayMicroseconds(durationMicros);
  pwmWrite(0);
  redLED(0);
}

/// Report the CPU temperature in Celsius units. Calibration was
/// performed with some cold packs, and a multimeter with a 
/// thermocouple.
float getInternalTemperatureC() {
   analogReadRes(16);
   const double offset(398.3);
   const double gain(-0.00951);
   double reading(analogRead(temperatureSensor));
   return offset + gain * reading;
}

VoltageMonitor voltageMonitor;

long readMillivolts() {
   return voltageMonitor.getValue();
}

void sampleVoltage() {
   voltageMonitor.getSample();
}

/// Turns out by the time user code runs, it's too late to change any
/// of these clock registers.
///
/// From
/// http://forum.pjrc.com/threads/724-32-768-KHz-crystal-and-battery-for-Teensy-3-0-RTC-questions?highlight=RTC_TPR
/// The bit to use is RTC_SR_TIF, but you can't easily do this because
/// the startup code checks that bit and tries to initialize the
/// RTC. By the time your program is running, it's too late to
/// discover what that bit was before the startup code ran.
///
/// There is a startup_early_hook() function you could implement to
/// run your own code very early in the startup. Pretty much nothing
/// is initialized at that point. The most viable solution would be to
/// grab the value of RTC_SR and put it into a static variable. But
/// this code runs before variables are initialized, so you'll have to
/// use an attribute to put that variable into the .noinit section to
/// prevent the later startup code from overwriting it. Then you could
/// check that variable's RTC_SR_TIF bit once your program is
/// running. Or if you don't need to do much, you could perhaps do
/// everything you need in that extremely early startup code.... but
/// that's quite difficult because so much stuff ordinary programming
/// depends upon hasn't been initialized yet.
void describeRTC() {
  printLabelValueUnits("RTC_TSR ", RTC_TSR, " time in seconds");
  printLabelValueUnits("RTC_CR & RTC_CR_CLKO ",
		       !!(RTC_CR & RTC_CR_CLKO), " 1=gated");
  printLabelValueUnits("RTC_CR & RTC_CR_OSCE ",
		       !!(RTC_CR & RTC_CR_OSCE), " 1=enabled");
  printLabelValueUnits("RTC_SR & RTC_SR_TCE ",
		       !!(RTC_SR & RTC_SR_TCE), " 1=enabled");
  printLabelValueUnits("RTC_SR & RTC_SR_TIF ",
		       !!(RTC_SR & RTC_SR_TIF), " 1=invalid");
  printLabelValueUnits("RTC_SR & RTC_SR_TCE ",
		       !!(RTC_SR & RTC_SR_TCE), " 1=prescale enabled");
  printLabelValueUnits("RTC_TPR ", RTC_TPR, " prescaler");
}

/// Alas, all the flags I can find give no indication whether the RTC
/// external clock crystal is or is not present. The RTC prescaler
/// should increment approximately once every 31 microseconds, so we
/// sample the prescaler, wait 33 microseconds, and sample again. If
/// the value is unchanged, there is no RTC external crystal, but if
/// the prescaler value has changed, then there is.
///
/// The flags I looked at were the following:
///    !!(RTC_CR & RTC_CR_CLKO)
///    !!(RTC_CR & RTC_CR_OSCE)
///    !!(RTC_SR & RTC_SR_TCE)
///    !!(RTC_SR & RTC_SR_TIF)
///    !!(RTC_SR & RTC_SR_TCE)
bool hasRTC() {
  uint32_t prescaler(RTC_TPR);
  delayMicroseconds(33);
  return prescaler != RTC_TPR;
}

void initPorts() {
   // Pins connected to the mini54. Do this for low power.
   pinMode(18, INPUT_PULLUP);
   pinMode(19, INPUT_PULLUP);

  analogReference(INTERNAL);
  analogReadAveraging(32);
  analogReadRes(16);    // Full resolution

  ADC0_CFG1 =  ADC_CFG1_ADIV(3) // ADC Clock = 24MHz / 8
    | ADC_CFG1_MODE(3*0)        // 16 bit mode
    | ADC_CFG1_ADICLK(0);       // Use bus clock

  analogRead(ditPin);   // Allow calibration to complete

  // Linux:
  //    sudo cu -l /dev/tty.usbserial-* -s 9600
  // OS X (nee Mac OS X) circa 10.8.4
  //    sudo cu -l /dev/tty.usbmodem* -s 9600 --nostop --parity=none
  Serial.begin(9600);		// Does not block.

  pinMode(ledPin, OUTPUT);
  pinMode(beepPin, OUTPUT);

  pinMode(piezoTxP, OUTPUT);
  pinMode(piezoTxN, OUTPUT);

  pinMode(ledGreenN, OUTPUT);
  pinMode(ledGreenP, OUTPUT);
  pinMode(ledRedP, OUTPUT);
  pinMode(ledRedN, OUTPUT);
  digitalWrite(ledRedN, 0);
  digitalWrite(ledGreenN, 0);
  greenLED(1);
  
  calibrateTouch(ditPin);
  calibrateTouch(dahPin);
}

void pwmFrequency(int Hz) {
  analogWriteFrequency(piezoTxP, Hz);
  analogWriteFrequency(txPin, Hz);
  analogWriteFrequency(earphoneRightPin, Hz);
}

void pwmWrite(int duty) {
  analogWrite(piezoTxP, duty);
  analogWrite(txPin, duty);
  analogWrite(earphoneRightPin, duty);
}

void dWrite(int value) {
  digitalWrite(piezoTxP, value);
  digitalWrite(txPin, value);
  digitalWrite(earphoneRightPin, value);
}

void redLED(int value) {
  digitalWrite(ledRedP, value);
}

void greenLED(int value) {
  digitalWrite(ledGreenP, value);
}

uint16_t activityLight(0);

void toggleGreenLED() {
   ++activityLight;
   greenLED(!(activityLight % 6));
}

void toggleRedLED() {
   ++activityLight;
   redLED(!(activityLight % 6));
}

