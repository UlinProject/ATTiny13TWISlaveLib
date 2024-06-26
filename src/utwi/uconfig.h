#pragma once

#define I2C_SIZE_RECVBUFF		3+1 +12 // (3: len+crc+cmd)
#define I2C_SIZE_SENDBUFF		3+1 +12 // (3: len+crc+OK_CMD)
#define I2C_ADDR				0x61

#define I2C_SCL				PB4	// SCL
#define I2C_SDA				PB3	// SDA

#define I2C_ENDBYTE				0xFF   // always_out: 0xFF
#define I2C_PAUSE_WAITLEN		0xFF-1 // always_out: 0xFF-1
#define I2C_PAUSE_WAITCRC8RUN		0xFF-2 // always_out: 0xFF-1
#define I2C_PAUSE_WAITENDRUN		0xFF-3 // always_out: 0xFF-1
#define I2C_PAUSE_UNKWAIT		0xFF-4 // To save flash memory, you can use this type of locking, which eliminates the possibility of determining the locking type.

#define I2C_CHECKPOINT			0xFF-9 // always_out: 0xFF-1

#define I2C_INVALIDCRC			0xFF-10 // out: _len+crc+invalid
