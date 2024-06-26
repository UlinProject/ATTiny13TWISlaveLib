
#include "UShortTimer.h"

#define TFN_UST(args...) template<const uint8_t TRIG_PIN_0> \
	args UShortTimer<TRIG_PIN_0>

TFN_UST()::UShortTimer() {}

TFN_UST(inline void)::begin(void) {
	pinMode_OUTPUT(TRIG_PIN_0);
	this->clear();
	this->set(0); /* off and off */
}

TFN_UST(inline void)::set(const uint16_t new_t0_event_mil = 0) {
	this->t0_event_mil = new_t0_event_mil;
}

TFN_UST(const uint8_t)::pin_0(void) {
	return TRIG_PIN_0;
}

TFN_UST(inline void)::clear(void) {
	this->t0_mil = 0;
	
	digWrite_LOW(TRIG_PIN_0);
}

TFN_UST(inline void)::add_trig_pin0(void) {
	/* life hack, it is much easier to set the maximum number of trigger times for someone else than to change the state of the ports yourself. */
	this->t0_mil = this->t0_event_mil;
}

TFN_UST(void)::next(void) {
	if (this->t0_event_mil != 0) {
		if (this->t0_mil >= this->t0_event_mil) {
			/* Without any artificial delays, the processor will be woken up by the "twi" or "timer" command. */
			digWrite_HIGH(TRIG_PIN_0);
			//sleep_cpu();
			//sleep_cpu();
			//sleep_cpu();
			//digWrite_LOW(TRIG_PIN_0);
			
			//this->t0_mil = 0;
		}else {
			if (65536 > this->t0_mil) {
				this->t0_mil++;
			}
		}
	}
}
