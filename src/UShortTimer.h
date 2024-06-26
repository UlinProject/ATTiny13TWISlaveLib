#pragma once

#include <periph.h>
#include <stdint.h>
#include <avr/sleep.h>

template<const uint8_t TRIG_PIN_0>
class UShortTimer {
	public:
		uint16_t t0_mil; // x
		uint16_t t0_event_mil; // for reset, 0 -> timer_off
	
		UShortTimer();
		inline void begin(void);
		inline void set(const uint16_t new_t0_event_mil = 0);
		inline const uint8_t pin_0(void);
		inline void clear(void);
		inline void add_trig_pin0(void);
		void next(void);
};
