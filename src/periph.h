#pragma once

#include <avr/io.h>

#define pinMode_OUTPUT(pin) 			DDRB |= _BV(pin)
//#define pinMode_2OUTPUT(pin, pin2)	DDRB |= (_BV(pin) | _BV(pin2)) /* this version eats more :( */
#define pinMode_INPUT(pin)			DDRB &= ~_BV(pin)
//#define pinMode_2INPUT(pin, pin2)	DDRB &= ~(_BV(pin) | _BV(pin2)) /* this version eats more :( */

#define pinMode_INPUT_PULLUP(pin)		pinMode_INPUT(pin); digWrite_HIGH(pin)
#define pinMode_DIS_INPUT_PULLUP(pin)	digWrite_LOW(pin)

#define digWrite_HIGH(pin)			PORTB |= _BV(pin)
#define digWrite_LOW(pin)			PORTB &= ~_BV(pin)
#define digWrite_INVERT(pin)			PORTB ^= _BV(pin)

#define digRead(pin)				!!(PINB & _BV(pin))
//#define digRead(pin)				((PINB >> pin) & 1) /* this version eats more :( */

#define _ALWAYS_INLINE				__attribute__( ( always_inline ) ) inline
#define _NEVER_INLINE				__attribute__( ( noinline ) )

#define F_CPU_UNK					0x0
#define F_CPU_128KHZ 				0x1
#define F_CPU_600KHZ				0x2
#define F_CPU_1_2MHZ				0x3
#define F_CPU_4_8MHZ				0x4
#define F_CPU_9_6MHZ				0x5

void _ALWAYS_INLINE _init_defsleep();
void _ALWAYS_INLINE _init_osccal();
const uint8_t _ALWAYS_INLINE get_cuint8_short_fcpu(void);