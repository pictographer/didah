#include <climits>
#include "teensy3_morse_bsp.h"
#include "MK20DZ10.h"

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
  uint8_t pinTable[] = {32, 0, 48, 64, 80, 81, 100, 104, 0, 144, 196, 0, 256 };
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
  uint8_t pfTable[] = { 128, 0, 256, 0, 512, 0, 0, 0, 512 };
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
	Serial.println(MC_SRSL); //XXX is zero on my T3

  cpuid[i] = 0;
  return cpuid;  
}

// PWM pins 3, 4, 5, 6, 9, 10, 20, 21, 22, 23

// touchRead pins 0, 1, 15, 16, 17, 18, 19, 22, 23
static uint16_t touchBase[24];
uint16_t touchMargin(60);

// Measure the noise floor for the given touch pin. A threshold is set
// 600pF above the maximum measured value.
void calibrateTouch(uint8_t pin) {
  touchBase[pin] = 0;
  for (int i(0); i < 10; ++i) {
    uint16_t pF20(touchRead(pin));
    analogWrite(beepPin, pF20);
    touchBase[pin] = touchBase[pin] < pF20 ? pF20 : touchBase[pin];
    delay(100);
  }
  Serial.print("Touch pin ");
  Serial.print(pin);
  Serial.print(" threshold: ");
  Serial.println(touchBase[pin]);
}

// Poll the given capacitive sensor pin. Return true if the value
// is greater than the callibration threshold for the pin.
bool touchPoll(uint8_t pin) {
  uint16_t pF20(touchRead(pin));
  return touchBase[pin] + touchMargin < pF20;
}

size_t duty(duty50);

size_t getDuty() {
  return duty;
}

void setDuty(uint8_t dutyCycle) {
  duty = dutyCycle;
}

// Output a square wave on pin 10 at the given frequency for the given duration.
void beep(uint32_t frequency, uint32_t durationMicros) {
  analogWriteFrequency(beepPin, frequency);
  analogWrite(beepPin, duty);
  delayMicroseconds(durationMicros);
  analogWrite(beepPin, 0);
}

