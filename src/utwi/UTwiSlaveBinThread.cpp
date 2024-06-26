
#include "uconfig.h"
#include "periph.h"
#include "UTwiSlaveBinThread.h"
// #UlinProject23

#define TFN_UTSBT(args...) template<const uint8_t SIZE_BUFF> \
	args UTwiSlaveBinThread<SIZE_BUFF>

TFN_UTSBT()::UTwiSlaveBinThread(void) {}

TFN_UTSBT(inline void)::begin(void) {
	/*
		It is not safe to read uninitialized memory, but this can be ignored for the sake of flash memory, because as long as len = 0, we will not read more than necessary.
	*/
	#ifdef I2C_INIT_ZEROMEM // see config
		memset(this->array, 0, sizeof(this->array));
	#endif
	
	//this->clear();
	this->_len = 0;
	this->t_read_pos = 0;
	this->t_write_pos = 0;
	//this->_pause_byte = 0;
}

TFN_UTSBT(inline void)::clear(void) {
	this->_len = 0;
}

TFN_UTSBT(const uint8_t)::write(const uint8_t a) {
	if (this->_len >= sizeof(this->array)) {
		// return 255; // in case of an error, it can damage the entire memory, so it is better to reset.
		return 0; // It makes more sense to damage the flow.
	}
	this->_len += 1;
	
	const uint8_t pos = this->t_write_pos;
	this->t_write_pos += 1;
	if (this->t_write_pos >= sizeof(this->array)) {
		this->t_write_pos = 0;
	}
	this->array[pos] = a;
	
	return pos;
}

TFN_UTSBT(const uint8_t)::current(void) {
	if (this->_len == 0) {
		return I2C_ENDBYTE;
	}
	return this->array[this->t_read_pos];
}

TFN_UTSBT(const uint8_t)::read(void) {
	if (this->_len == 0) {
		return I2C_ENDBYTE;
	}
	this->_len -= 1;
	
	uint8_t result = this->array[this->t_read_pos];
	this->t_read_pos += 1;
	if (this->t_read_pos >= sizeof(this->array)) {
		this->t_read_pos = 0;
	}
	
	return result;
}

TFN_UTSBT(inline void)::skip(void) {
	/*const uint8_t _skip = */this->read(); // It's best to do it this way for the sake of size)
}

TFN_UTSBT(inline uint8_t)::available(void) {
	return this->_len;
}

TFN_UTSBT(_ALWAYS_INLINE const uint8_t)::size(void) {
	return SIZE_BUFF;
}

TFN_UTSBT(inline const uint8_t)::len(void) {
	return this->_len;
}

TFN_UTSBT(inline bool)::is_empty(void) {
	return this->_len == 0;
}

TFN_UTSBT(inline void)::unchecked_setpos(const uint8_t pos, const uint8_t v) {
	this->array[pos] = v;
}

TFN_UTSBT(inline void)::unchecked_setlen(const uint8_t newlen) {
	this->_len = newlen;
}

TFN_UTSBT(uint8_t)::make_crc8() {
	/*Even if we replace everything with operations of reading and saving the position of the stream, the result will be a couple of bytes larger :)*/
	uint8_t len = this->len();
	uint8_t i = this->t_read_pos;
	uint8_t a;
	uint8_t result = 0;
	
	while (len > 0) {
		len--;
		
		a = this->array[i++];
		if (i >= sizeof(this->array)) {
			i = 0;
		}
		uint8_t counter;
		uint8_t buffer;
		
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
			: [crc_out]"=r" (result), [counter]"=d" (counter), [buffer]"=d" (buffer)
			: [crc_in]"0" (result), [data_in]"r" (a)
		);
	};
	
	return result;
}
