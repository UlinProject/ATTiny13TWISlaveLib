#pragma once
// #UlinProject23

/* CPU */
// https://raw.githubusercontent.com/MCUdude/MicroCore/master/avr/libraries/Serial_examples/examples/OscillatorCalibration/OscillatorCalibration.ino
// If you can’t calibrate, then add something initial approximate to the main one (CalibrationUtil) OSCCAL = 0x20. 0x20 helped me and then I was able to calibrate as needed.
// OSCCAL = 0x40;
//#define CPU_SUP_EE_CAL

// out:
// busybox microcom -t 115200 /dev/ttyUSB0
//
// New OSCCAL stored to EEPROM addr. 0
#define CPU_FIX_CAL 			0x59 /* simply entering a value here requires much less memory. */
// 0x48?
#define FIRMWARE_VER 			0x01

#define PIN_TIMER0				PB0
#define PIN_TIMER1				PB1
// PB2
// PB5 - RESET
//#define I2C_USERMEMSIZE		6

#define I2C_CMD_RESULT_ERR		0xFF-11 // out: _len+crc+C_ERR
#define I2C_CMD_RESULT_OK		0xFF-12 // out: _len+crc+C_OK
#define I2C_CMD_RESULT_UNK		0xFF-13 // out: _len+crc+C_UNK
#define I2C_CMD_INF				0xFF-14 // out: _len+crc+FIRMWARE_VER+I2C_USERMEMSIZE+S_RECVBUFF+S_SENDBUFF

/*#define I2C_CMD_PIN_GET		0xFF-20
#define I2C_CMD_PIN_HIGH			0xFF-21
#define I2C_CMD_PIN_LOW			0xFF-22*/

/*#define I2C_CMD_USERARRAYMEM_GET	0xFF-30
#define I2C_CMD_USERARRAYMEM_SET	0xFF-31*/
#define I2C_CMD_USERONEMEM_GET	0xFF-32
#define I2C_CMD_USERONEMEM_SET	0xFF-33

#define I2C_CMD_TIME_INF			0xFF-40
#define I2C_CMD_TIME_SETANDRESET	0xFF-41
#define I2C_CMD_TIME_RESET		0xFF-42
#define I2C_CMD_TIME_TRIG_PIN0	0xFF-43
#define I2C_CMD_TIME_TRIG_PIN1	0xFF-44
