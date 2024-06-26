#pragma once

#include <avr/io.h>

#define pinMode_OUTPUT(pin) 			DDRB |= _BV(pin)
#define pinMode_INPUT(pin)			DDRB &= ~_BV(pin)

#define pinMode_INPUT_PULLUP(pin)		pinMode_INPUT(pin); digWrite_HIGH(pin)
#define pinMode_DIS_INPUT_PULLUP(pin)	digWrite_LOW(pin)

#define digWrite_HIGH(pin)			PORTB |= _BV(pin)
#define digWrite_LOW(pin)			PORTB &= ~_BV(pin)
#define digWrite_INVERT(pin)			PORTB ^= _BV(pin)

#define digRead(pin)				!!(PINB & _BV(pin))

#define _ALWAYS_INLINE				__attribute__( ( always_inline ) ) inline
#define _NEVER_INLINE				__attribute__( ( noinline ) )
