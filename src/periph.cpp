
#include "periph.h"
#ifdef CPU_SUP_EE_CAL
	#include <EEPROM.h>
#endif

void _ALWAYS_INLINE _init_defsleep() {
	ACSR &= ~(1 << ACD); // Analog comparator off
	ADCSRA &= ~(1 << ADEN); // ADC Off
	
	//ACSR &= ~(1 << ACIE); // Analog comparator INT off
	//ADCSRA &= ~(1 << ADIE); // ADC INT OFF
	//GIMSK &= ~(1 << INT0); // INT0 INT OFF
	//TIMSK0 &= ~(1 << TOIE0); // TIMER0 INT OFF
}

void _ALWAYS_INLINE _init_osccal() {
	#ifdef CPU_FIX_CAL
		OSCCAL = CPU_FIX_CAL;
	#endif
	
	// __load_eoscall_calib
	#ifdef CPU_SUP_EE_CAL
		EEPROM.begin();
		const uint8_t cal = EEPROM.read(0);
		if (cal != 0x0 && cal < 0x7F) {
			OSCCAL = cal;
		}
	#endif
}

const uint8_t _ALWAYS_INLINE get_cuint8_short_fcpu(void) {
	/* saving memory on bit shifts, just represent small possible values. */
	switch (F_CPU) {
		case 128000:
			return F_CPU_128KHZ;
			break;
		case 600000:
			return F_CPU_600KHZ;
			break;
		case 1200000:
			return F_CPU_1_2MHZ;
			break;
		case 4800000:
			return F_CPU_4_8MHZ;
			break;
		case 9600000:
			return F_CPU_9_6MHZ;
			break;
		default:
			return F_CPU_UNK;
			break;
	}
}
