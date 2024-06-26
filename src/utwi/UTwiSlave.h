#pragma once
// #UlinProject23

#include <stdint.h>
#include "uconfig.h"
#include "ucore.h"
#include "UTwiSlaveBinThread.cpp"
#include "UTwiSlavePos.h"

#define GET_SDA()			digRead(I2C_SDA)
#define GET_SCL()			digRead(I2C_SCL)
#define IN_SDA()			pinMode_INPUT(I2C_SDA)
#define OUT_SDA()			pinMode_OUTPUT(I2C_SDA)
#define OUT_SDA_SCL()		pinMode_2OUTPUT(I2C_SDA, I2C_SCL)
#define LOW_SDA()			digWrite_LOW(I2C_SDA)
#define HIGH_SDA()			digWrite_LOW(I2C_SDA)

#define IN_SCL()			pinMode_INPUT(I2C_SCL)
#define OUT_SCL()			pinMode_OUTPUT(I2C_SCL)
#define LOW_SCL()			digWrite_LOW(I2C_SCL)

class UTwiSlave {
	UTwiSlaveBinThread<I2C_SIZE_SENDBUFF> send;
	uint8_t send_crc;
	uint8_t _pause_byte; // only for i2c_int
	UTwiSlaveBinThread<I2C_SIZE_RECVBUFF> recv;
	
	public:
		inline void begin(void);
		inline void begin_pins(void);
		
		inline void begin_pcieint(void);
		_ALWAYS_INLINE const bool is_int(void);
		inline void dis_pcieint(void);
		
		inline void end(void);
		inline void loop(void);
		
		inline void clear(void);
		inline void wclear(void);
		inline void rclear(void);
		
		inline uint8_t get_wcrc(void);
		inline uint8_t write_skipcrc8(const uint8_t v);
		inline void reserve_len_and_crc8(UTwiSlavePos &pos);
		inline void write_len_and_crc8(const UTwiSlavePos &pos);
		inline const uint8_t write(const uint8_t v);
		inline uint8_t available_write(void);
		inline uint8_t available(void);
		inline const bool is_empty(void);
		inline const uint8_t read(void);
		inline const uint8_t current(void);
		inline const uint8_t rmax(void);
		inline void skip(void);
		
		inline uint8_t make_read_crc(void);
		
		//inline void set_rint_pause(const uint8_t pausebyte = 0);
		//inline void set_wint_pause(const uint8_t pausebyte = 0);
		
		inline void set_int_pause(const uint8_t pausebyte = 0);
		inline void skip_int_pause(void);
		
		inline bool is_wflush(void);
		inline void next(void);
};

