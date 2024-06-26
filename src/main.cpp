
#include "config.h" // see config
#include "periph.cpp"
#include <avr/interrupt.h>
//#include <avr/wdt.h>
#include <Arduino.h>
#include "UShortTimer.cpp"
#include <avr/sleep.h>
#include <stdlib.h>
#include "utwi/UTwiSlave.cpp"

volatile UTwiSlave twi;
volatile UShortTimer<PIN_TIMER0> timer;
ISR(PCINT0_vect) {
	if(twi.is_int()) {
		twi.next();
	}
	// my library allows you to use twi both inside the pcient interrupt and inside the main code (but then there is a chance to miss the request)
	// if you are using a lot of pins for input, make sure it doesn't affect twi, in which case use an int interrupt on a specific pin.
}

ISR(TIM0_COMPA_vect) {
	timer.next();
}

/* short, meaningful commands for greater code clarity and less flash memory */
#define _WRITE_ONEBYTE(byte)		_PREP_U8(byte); goto _SEND_ONE_BYTE
#define _SEND_OK()				goto _SEND_OK
#define _SEND_ERR()				goto _SEND_ERR
#define _SEND_UNK()				goto _SEND_UNK
/* to save fmemory and everything works. */
#define _PREP_U8(args)			v_arg = args;
#define _RET_U8				v_arg
#define _PREP_U16(args)			v_arg3 = args;
#define _RET_U16				v_arg3
#define _PREP_U8_2(args)			v_arg2 = args;
#define _RET_U8_2				v_arg2

int main() {
	//wdt_reset(); // in general, it can be deleted, but I leave it here in case of a possible error with wdt.
	//wdt_enable(WDTO_8S); // strange bug, if you turn on wdt once, you can’t turn it off :)
	cli();
	_init_defsleep();
	_init_osccal();
	
	// I2C
	twi.begin();
	twi.begin_pcieint();
	
	// TIMERS
	timer.begin();
	TCCR0B = 0; /*0 - off*/
	//TIMSK0 |= (1 << TOIE0); /* en int */
	TCCR0A |= (1 << WGM01);
	TIMSK0 |= (1 << OCIE0A);
	OCR0A = 255;
	
	/**/
	//set_sleep_mode(SLEEP_MODE_PWR_DOWN); /* SLEEP_MODE_IDLE - allows you to wake up the processor from a timer */
	//sleep_enable();
	
	
	uint8_t v_arg;
	uint8_t v_arg2;
	uint16_t v_arg3;
	//uint8_t usermem[I2C_USERMEMSIZE];
	uint8_t useronemem = 0x1;
	UTwiSlavePos lcrc8;
	
	sei();
	while(true) {
		//wdt_reset();
		while (!twi.is_empty()) { // len>0!
			_PREP_U8(twi.current()); // if there are no bytes, then I2C_ENDBYTE will be returned, even if it is 255, this does not violate the condition in any way.
			if (_RET_U8 > twi.rmax()) {
				// so as not to get stuck in an eternal loop due to a software error on the master.
				
				twi.skip(); // del_len
				continue;
			}
			
			_PREP_U8(_RET_U8 +1+1); // 1+1: len+crc8
			if (twi.available() >= _RET_U8) {
				twi.set_int_pause(I2C_PAUSE_UNKWAIT);
				twi.skip(); /* del_len */
				
				/* skip_and_read: del_len+read crc8 */
				// CRC8: DATA (-LEN, -CRC8)
				_PREP_U8(twi.read()); // crc8
				if (_RET_U8 == twi.make_read_crc()) { // 1symb - crc
					// START
					//twi.set_int_pause(I2C_PAUSE_WAITENDRUN);
					twi.reserve_len_and_crc8(lcrc8);
					while(true) {
						_PREP_U8(twi.read());
						switch (_RET_U8) { /* C_CMD */
							case I2C_ENDBYTE: //end
								// read will return I2C_ENDBYTE if there are no bytes left. This can also be useful if you need to further clear the entire queue.
								twi.clear(); // recv clear, You can't clear the whole stream, we're not alone here
								
								goto _END_LOOP_CMD; // end, break+break:)
								break;
							/*case I2C_CMD_USERARRAYMEM_SET:
								_PREP_U8(twi.read()); // ADDR
								_PREP_U8_2(twi.read());
								if (_RET_U8 > sizeof(usermem)) {
									_PREP_U8(I2C_CMD_RESULT_ERR);
								} else {
									usermem[_RET_U8] = _RET_U8_2;
									_PREP_U8(I2C_CMD_RESULT_OK);
								}
								//goto _SEND_ONE_BYTE;
							case I2C_CMD_USERARRAYMEM_GET:
								_PREP_U8(v_arg = twi.read()); // ADDR
								if (v_arg > sizeof(usermem)) {
									_PREP_U8(I2C_CMD_RESULT_ERR);
								} else {
									_PREP_U8(usermem[_RET_U8]);
								}
								
								goto _SEND_ONE_BYTE;*/
							/*case I2C_CMD_PIN_GET:
								_PREP_U8(twi.read()); // PIN
								pinMode_INPUT(_RET_U8);
								_PREP_U8(digRead(_RET_U8)); // 
								
								_WRITE_ONEBYTE(_RET_U8);*/
							/*case I2C_CMD_PIN_HIGH:
								_PREP_U8(twi.read());
								pinMode_OUTPUT(v_arg);
								digWrite_HIGH(v_arg);
								
								_SEND_OK();
							case I2C_CMD_PIN_LOW:
								_PREP_U8(twi.read());
								pinMode_OUTPUT(_RET_U8);
								digWrite_LOW(_RET_U8);
								
								_SEND_OK();*/
							//case I2C_CMD_INF:
								//_WRITE_ONEBYTE(FIRMWARE_VER);
								//twi.write(sizeof(usermem));
								//twi.write(0); // usermem
								//twi.write(1); // useronemem
								
								//twi.write(I2C_SIZE_RECVBUFF);
								//_WRITE_ONEBYTE(I2C_SIZE_SENDBUFF);
							case I2C_CMD_TIME_INF:
								twi.write(get_cuint8_short_fcpu());
								twi.write(TCCR0B);
								twi.write(TCNT0);
								twi.write(OCR0A);
							
								twi.write(timer.pin_0());
								
								
								twi.write((timer.t0_mil >> 8) & 0xFF);
								twi.write(timer.t0_mil & 0xFF);
								
								twi.write((timer.t0_event_mil >> 8) & 0xFF);
								_WRITE_ONEBYTE(timer.t0_event_mil & 0xFF);
								// 9 bytes
								break;
							case I2C_CMD_TIME_SETANDRESET:
								TCCR0B = 0; /* timer off */
								_PREP_U8(twi.read());
								_PREP_U16(_RET_U8 << 8 | twi.read());
								timer.set(_RET_U16);
								
								_PREP_U8(twi.read());
								OCR0A = _RET_U8;
								
								_PREP_U8(twi.read()); // divider
								TCCR0B = _RET_U8;
								//sei();
								
								goto _I2C_CMD_TIME_RESET; /* In addition to installation, we also do cleaning, cheaper with goto. */
								break;
							case I2C_CMD_TIME_RESET:
								_I2C_CMD_TIME_RESET:
									TCNT0 = 0;
									timer.clear();
									
									_SEND_OK();
								break;
							/*case I2C_CMD_TIME_TRIG_PIN0:
								timer.add_trig_pin0();
								
								_SEND_OK();
								break;*/
							/*case I2C_CMD_TIME_TRIG_PIN1:
								timer.add_trig_pin1();
								
								_SEND_OK();*/
							case I2C_CMD_USERONEMEM_GET:
								_WRITE_ONEBYTE(useronemem);
								
								break;
							case I2C_CMD_USERONEMEM_SET:
								useronemem = twi.read();
								
								_SEND_OK();
								break;
							/*case I2C_CMD_TIME_TRIG_PIN1:
								v_arg = PIN_TIMER1;
								goto _TIMER_TRIG;*/
							default:
								_SEND_UNK();
								break;
						}
						continue;
						
						_SEND_OK:
							_WRITE_ONEBYTE(I2C_CMD_RESULT_OK);
						/*_SEND_ERR:
							_WRITE_ONEBYTE(I2C_CMD_RESULT_ERR);*/
						_SEND_UNK:
							_WRITE_ONEBYTE(I2C_CMD_RESULT_UNK);
						_SEND_ONE_BYTE:
							twi.write(_RET_U8);
							continue;
					}
					_END_LOOP_CMD:
						twi.write_len_and_crc8(lcrc8);
				}else {
					twi.write(I2C_INVALIDCRC);
					//twi.write(_RET_U8); // current_crc
				}
				twi.skip_int_pause();
			}
		}
		
		//sleep_cpu();
	}
}
