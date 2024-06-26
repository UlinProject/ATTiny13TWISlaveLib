
#include "UTwiSlave.h"
// #UlinProject23
// UTwiSlave::UTwiSlave() {}

inline void UTwiSlave::end(void) {}

inline void UTwiSlave::begin(void) {
	this->begin_pins();
	
	this->send.begin();
	this->recv.begin();
	this->send_crc = 0;
	this->skip_int_pause();
}

inline void UTwiSlave::begin_pins(void) {
	pinMode_INPUT(I2C_SDA);
	pinMode_INPUT(I2C_SCL);
	//pinMode_2INPUT(I2C_SDA, I2C_SCL);
}

inline void UTwiSlave::begin_pcieint(void) {
	GIMSK |= _BV(PCIE);
	PCMSK |= _BV(I2C_SDA); // name_pin = name_int
}

inline void UTwiSlave::dis_pcieint(void) {
	GIMSK &= ~_BV(PCIE);
	PCMSK &= ~_BV(I2C_SDA); // name_pin = name_int
}

_ALWAYS_INLINE const bool UTwiSlave::is_int(void) {
	// EVENT_START
	return !GET_SDA() && GET_SCL();
}

inline void UTwiSlave::loop(void) {
	while (true) {
		while (!this->is_int()) {}
		this->next();
	}
}

inline void UTwiSlave::wclear(void) {
	this->send.clear();
	this->send_crc = 0;
}

inline void UTwiSlave::rclear(void) {
	this->recv.clear();
}

inline uint8_t UTwiSlave::get_wcrc(void) {
	return this->send_crc;
}

inline uint8_t UTwiSlave::write_skipcrc8(const uint8_t v) {
	return this->send.write(v);
}

inline void UTwiSlave::reserve_len_and_crc8(UTwiSlavePos &pos) {
	/* It's easier to write something down to save space. */
	pos.poslen = this->write_skipcrc8(I2C_CHECKPOINT);
	pos.poscrc = this->write_skipcrc8(I2C_CHECKPOINT);
}

inline void UTwiSlave::write_len_and_crc8(const UTwiSlavePos &pos) {
	//const uint8_t len = this->send.len();
	// saving memory, possible overflow if the order of functions is performed incorrectly
	//if (len >= 2) {
		this->send.unchecked_setpos(pos.poslen, this->send.len()-2/*len -2*/);
		this->send.unchecked_setpos(pos.poscrc, this->send_crc);
		
		this->send_crc = 0;
	//}
}

const uint8_t UTwiSlave::write(const uint8_t v) {
	const uint8_t pos = this->write_skipcrc8(v);
	uint8_t counter;
	uint8_t buffer;
	
	/* CRC8 */
	asm volatile (
		"EOR %[crc_out], %[data_in] \n\t"
		"LDI %[counter], 8          \n\t"
		"LDI %[buffer], 0x8C        \n\t"
		"_loop_start_%=:            \n\t"
		"LSR %[crc_out]             \n\t"
		"BRCC _loop_end_%=          \n\t"
		"EOR %[crc_out], %[buffer]  \n\t"
		"_loop_end_%=:              \n\t"
		"DEC %[counter]             \n\t"
		"BRNE _loop_start_%="
		: [crc_out]"=r" (this->send_crc), [counter]"=d" (counter), [buffer]"=d" (buffer)
		: [crc_in]"0" (this->send_crc), [data_in]"r" (v)
	);
	
	return pos;
}

inline uint8_t UTwiSlave::available_write(void) {
	return this->send.available();
}

inline uint8_t UTwiSlave::available(void) {
	return this->recv.available();
}

inline const bool UTwiSlave::is_empty(void) {
	return this->available() == 0;
}

inline const uint8_t UTwiSlave::read(void) {
	return this->recv.read();
}

inline const uint8_t UTwiSlave::current(void) {
	return this->recv.current();
}

inline const uint8_t UTwiSlave::rmax(void) {
	return this->recv.size();
}

inline void UTwiSlave::skip(void) {
	this->recv.skip();
}

inline uint8_t UTwiSlave::make_read_crc(void) {
	return this->recv.make_crc8();
}

/*inline void UTwiSlave::set_rint_pause(const uint8_t pausebyte) {
	this->recv.set_int_pause(pausebyte);
}

inline void UTwiSlave::set_wint_pause(const uint8_t pausebyte) {
	this->send.set_int_pause(pausebyte);
}*/

/* void UTwiSlave::set_int_pause(const uint8_t pausebyte) {
	this->set_wint_pause(pausebyte);
}*/

inline void UTwiSlave::set_int_pause(const uint8_t pausebyte) {
	//this->set_wint_pause(pausebyte);
	//this->set_rint_pause(pausebyte);
	this->_pause_byte = pausebyte;
}

inline void UTwiSlave::skip_int_pause(void) {
	//this->set_wint_pause(0);
	//this->set_rint_pause(0);
	this->_pause_byte = 0;
}


inline void UTwiSlave::clear(void) {
	this->recv.clear();
}

/*inline void UTwiSlave::skip_int_pause(void) {
	this->set_int_pause(0);
}*/

inline bool UTwiSlave::is_wflush(void) {
	return this->send.is_empty();
}

inline void UTwiSlave::next(void) {
	uint8_t index;
	uint8_t a;
	
	// to save interrupt time the code is placed here
	
	_START_EVENT:
		//IN_SCL(); // clock_stretching stop
		//while (GET_SDA()) {} // START_EVENT
		if (!this->is_int()) {
			return;
		}
		
		a = 0;
		index = 0;
		while (GET_SCL()) {} //
		while (index++ < 8) {
			a = (a << 1);
			while(!GET_SCL());
			if (GET_SDA()) a |= 0x1;
			
			while(GET_SCL()) { // DETECT START/STOP EVENT
				if((a & 1) != GET_SDA()) {
					if(GET_SDA()) {
						// STOP
						goto _STOP_EVENT;
					}
					// START
					goto _START_EVENT;
				}
			}
		}
		
		if ((a & ~1) == (I2C_ADDR << 1)) {
			OUT_SDA();
			while(!GET_SCL());
			while(GET_SCL());
			IN_SDA();
			//LOW_SDA();
			//while(GET_SCL());
			OUT_SCL(); // clock_stretching start
			//LOW_SCL();
			//IN_SDA();
			
			if(a & 1) {
				// tiny => master
				while(true) {
					// it works now, but if you change it, there may be problems with i2c
					//a = this->send.get_int_pause();
					a = this->_pause_byte;
					if (a == 0) {
						a = this->send.read();
					}
					
					//while(!GET_SCL()) {}
					for(index = 0; index < 8; index++) {
						while(GET_SCL()) {}
						
						//if (a & (1 << index)) {
						if((a >> (7 - index)) & 1) {
							IN_SDA();
						} else {
							OUT_SDA();
						}
						
						IN_SCL(); // clock_stretching stop
						while(!GET_SCL()) {}
					}
					while(GET_SCL()) {}
					
					IN_SDA();

					while(!GET_SCL()) {}
					if(!GET_SDA()) {
						continue;
					}else {
						// H_SCL + H_SDA
						return;
					}
				}
				
			} else {
				// master => tiny
				while(true) {
					a = 0;
					
					IN_SCL(); // clock_stretching stop
					//while (!GET_SCL()){}
					for(index = 0; index < 8; index++) {
						a = (a << 1);
						while(!GET_SCL());
						if (GET_SDA()) a |= 0x01;
						while(GET_SCL()) { // DETECT START/STOP EVENT
							if((a & 1) != GET_SDA()) {
								if(GET_SDA()) {
									// _TWI_SLA_STOP
									goto _STOP_EVENT;
								}
								goto _START_EVENT;
							}
						}
					};
					OUT_SDA();
					while(!GET_SCL()) {}
					while(GET_SCL()) {}
					IN_SDA();
					
					//is_send_ask = false;
					if (this->_pause_byte == 0) {
					//if (this->recv.get_int_pause() == 0) {
						this->recv.write(a);
					//}
					}
					
					/*if(!GET_SDA()) {
						continue;
					}else {
						return;
					}*/
				}
				
			}
		}
		return;
	_STOP_EVENT:
		//while (GET_SCL()) {}
		//_delay_us(10);
		return;
}
