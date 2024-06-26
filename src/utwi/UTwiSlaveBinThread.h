#pragma once
// #UlinProject23

#include "uconfig.h"
#include <stdint.h>

template<const uint8_t SIZE_BUFF>
class UTwiSlaveBinThread {
	uint8_t array[SIZE_BUFF];
	uint8_t _len;
	uint8_t t_read_pos;
	uint8_t t_write_pos;
	
	public:
		UTwiSlaveBinThread(void);
		
		inline const uint8_t len();
		inline void begin(void);
		
		inline void clear(void);
		
		const uint8_t write(const uint8_t a);
		inline void unchecked_setpos(const uint8_t pos, const uint8_t v);
		inline void unchecked_setlen(const uint8_t newlen);
		
		inline uint8_t available(void);
		_ALWAYS_INLINE const uint8_t size(void);
		inline bool is_empty(void);
		
		const uint8_t current(void);
		const uint8_t read(void);
		inline void skip(void);
		
		uint8_t make_crc8(void);
};
