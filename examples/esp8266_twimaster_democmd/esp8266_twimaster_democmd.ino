
// #UlinProject23

#include <Wire.h>
extern "C" {
#include "gpio.h"
}
extern "C" {
#include "user_interface.h"
}

#define GPIO0 0
#define GPIO1 1
#define GPIO2 2
#define GPIO3 3
#define GPIO4 4
#define GPIO5 5
#define GPIO6 6
#define GPIO7 7
#define GPIO8 8
#define GPIO9 9
#define GPIO10 10
#define GPIO11 11
#define GPIO12 12
#define GPIO13 13
#define GPIO14 14
#define GPIO15 15
#define GPIO16 16

#define D0 GPIO16
#define D1 GPIO5
#define D2 GPIO4
#define D3 GPIO0
#define D4 GPIO2
#define D5 GPIO14
#define D6 GPIO12
#define D7 GPIO13
#define D8 GPIO15
#define D9 GPIO3
#define D10 GPIO1

#define I2C_ENDBYTE        0xFF   // always_out: 0xFF
#define I2C_PAUSE_WAITLEN   0xFF-1 // always_out: 0xFF-1
#define I2C_PAUSE_WAITCRC8RUN   0xFF-2 // always_out: 0xFF-1
#define I2C_PAUSE_WAITENDRUN    0xFF-3 // always_out: 0xFF-1
#define I2C_PAUSE_UNKWAIT   0xFF-4 // To save flash memory, you can use this type of locking, which eliminates the possibility of determining the locking type.
#define I2C_CHECKPOINT      0xFF-9 // always_out: 0xFF-1
#define I2C_INVALIDCRC      0xFF-10 // out: _len+crc+invalid
#define I2C_CMD_RESULT_ERR    0xFF-11 // out: _len+crc+C_ERR
#define I2C_CMD_RESULT_OK   0xFF-12 // out: _len+crc+C_OK
#define I2C_CMD_RESULT_UNK    0xFF-13 // out: _len+crc+C_UNK
#define I2C_CMD_INF       0xFF-14 // out: _len+crc+FIRMWARE_VER+I2C_USERMEMSIZE+S_RECVBUFF+S_SENDBUFF
/*#define I2C_CMD_PIN_GET     0xFF-20
#define I2C_CMD_PIN_HIGH      0xFF-21
#define I2C_CMD_PIN_LOW     0xFF-22*/
/*#define I2C_CMD_USERARRAYMEM_GET  0xFF-30
#define I2C_CMD_USERARRAYMEM_SET  0xFF-31*/
#define I2C_CMD_USERONEMEM_GET  0xFF-32
#define I2C_CMD_USERONEMEM_SET  0xFF-33
#define I2C_CMD_TIME_INF      0xFF-40
#define I2C_CMD_TIME_SETANDRESET      0xFF-41
#define I2C_CMD_TIME_RESET    0xFF-42
#define I2C_CMD_TIME_TRIG_PIN0  0xFF-43
//#define I2C_CMD_TIME_TRIG_PIN1  0xFF-44

#define F_CPU_UNK               0x0
#define F_CPU_128KHZ        0x1
#define F_CPU_600KHZ        0x2
#define F_CPU_1_2MHZ        0x3
#define F_CPU_4_8MHZ        0x4
#define F_CPU_9_6MHZ        0x5

#define I2C_ADDR 0x61

class SkipSerial {
  public:
    inline void print() {}
    inline void print(String str) {}
    inline void print(const uint8_t&, int = 0) {}
    inline void print(const uint32_t&, int = 0) {}
    inline void print(const int8_t&, int = 0) {}
    inline void println() {}
    inline void println(String str) {}
    inline void println(const uint8_t&, int = 0) {}
    inline void print(const int32_t&, int = 0) {}
    inline void println(const int8_t&, int = 0) {}
};

SkipSerial skip_serial;
#define I2CSerialDebug skip_serial
//#define I2CSerialDebug Serial

class TRecvLenCrcResult {
  public:
    uint8_t len;
    uint8_t crc8;
  
    bool is_err;
    TRecvLenCrcResult(void) {
      this->is_err = false;
      this->len = 0;
      this->crc8 = 0;
    }
};

/*
divider
| 0 (No clock)      | 000                            |
| 1                 | 001                            |
| 8                 | 010                            |
| 64                | 011                            |
| 256               | 100                            |
| 1024              | 101                            |
| External clock    | 110                            |
| External clock    | 111                            |
*/
enum class TTimerDivider: const uint8_t {
  P0 = 0b000,
  P1 = 0x01,
  P8 = 0x02,
  P64 = 0x03,
  P256 = 0x04,
  P1024 = 0x05,
  PinPB2Low = 0x06,
  PinPB2High = 0x07,
};

struct TFirmInfo {
  uint8_t FIRMWARE_VER;
  uint8_t I2C_SIZE_RECVBUFF;
  uint8_t I2C_SIZE_SENDBUFF;
  
  TFirmInfo() {}
};


struct TTimeInfo {
  uint32_t f_cpu;
  uint32_t divider;
  uint8_t current;
  uint8_t ifnextevent;
  
  uint8_t pin_0;
  uint16_t t0_mil;
  uint16_t t0_event_mil;

  TTimeInfo() {}
};

inline void make_onesymb_crc8(uint8_t &crc, uint8_t symb) {
  for (uint8_t j = 8; j > 0; j--) {
    crc = ((crc ^ symb) & 1) ? (crc >> 1) ^ 0x8C : (crc >> 1);
    symb >>= 1;
  }
}

const uint8_t make_crc8(const uint8_t cmd, const uint8_t (*buffer)[] = NULL, const uint8_t len = 0) {
  uint8_t crc = 0;

  make_onesymb_crc8(crc, cmd);
  for (uint8_t i = 0; i < len; i++) {
    const uint8_t data = (*buffer)[i];
    make_onesymb_crc8(crc, data);
  }
  
  return crc;
}


const bool t_send(const uint8_t cmd, const uint8_t (*send_array)[] = NULL, const uint8_t send_len = 0) {
  uint8_t c_attempts = 2;
  uint8_t send_error;
  uint8_t send_crc = make_crc8(cmd, send_array, send_len);
  uint8_t i;
  
  while (c_attempts > 0) {
    I2CSerialDebug.print(F("SEND[cmd: 0xFF-"));
    I2CSerialDebug.print(0xFF - cmd);
    I2CSerialDebug.print(F(", crc8: "));
    I2CSerialDebug.print(send_crc);
    I2CSerialDebug.print(F(") ("));
    
    i = 0;
    while (send_len > i) {
      if (i > 0) {
        I2CSerialDebug.print(F(" "));
      }
      I2CSerialDebug.print(F("0x"));
      I2CSerialDebug.print((uint8_t)(*send_array)[i], HEX);
      i++;
      yield();
    }
    I2CSerialDebug.println(F(");"));
    I2CSerialDebug.println(F("#[=================]#"));
  
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(send_len + 1); /*+1 - cmd*/
    Wire.write(send_crc);
    Wire.write(cmd);
  
    i = 0;
    while (send_len > i) {
      Wire.write((uint8_t)(*send_array)[i]);
      i++;
      yield();
    }
    send_error = Wire.endTransmission();
    if (send_error == 2) {
      I2CSerialDebug.println(F("err2: Device not found."));
      delay(200);
      yield();
      
      c_attempts -= 1;
      continue;
    }
    return true; // OK
  }
  
  return false;
}

TRecvLenCrcResult t_recv_lencrc() {
  TRecvLenCrcResult result;
  int8_t req_err;
  uint8_t c_req_attempt_req = 4;
  uint8_t c_req_attempt_tinywithoutint = 3;
  uint8_t c_req_attempt_pause = 5;
  //uint8_t c_req_attempt_invalidcrc = 4;

  while (c_req_attempt_req > 0 && c_req_attempt_pause > 0 /*&& c_req_attempt_invalidcrc != 0*/ && c_req_attempt_tinywithoutint > 0) {
    req_err = Wire.requestFrom(I2C_ADDR, (uint8_t)2);    // send_len+crc8
    yield();
    if (req_err < 2 || Wire.available() < 2) {
      I2CSerialDebug.print(F("InvalidReq2: "));
      I2CSerialDebug.println(req_err);
      delay(1);
      yield();
      c_req_attempt_req -= 1;
      continue;
    }
    
    const uint8_t req_len = Wire.read();
    const uint8_t req_crc8 = Wire.read();
  
    if (req_len == I2C_ENDBYTE) {
      c_req_attempt_tinywithoutint -= 1;
      I2CSerialDebug.println(F("I2C_ENDBYTE, The tiny may not have started yet, or there may be errors during data transfer."));
      delay(100);
      yield();
      
      continue;
    }else
    if (req_len == I2C_CHECKPOINT) {
      I2CSerialDebug.println(F("I2C_CHECKPOINT, Undefined behavior, setting length and crc8 were not created."));
      yield();
      
      result.is_err = true;
      return result;
    }else
    if(req_len == I2C_PAUSE_UNKWAIT || req_len == I2C_PAUSE_WAITENDRUN || req_len == I2C_PAUSE_WAITCRC8RUN || req_len == I2C_PAUSE_WAITLEN) {
      I2CSerialDebug.println(F("I2C_PAUSE, Tiny received the task and continues to carry it out. You need to rush him less."));
      yield();
      delay(10);
      c_req_attempt_pause -= 1;
      
      continue;
    }else
    if (req_len == I2C_INVALIDCRC) {
      I2CSerialDebug.println("Tiny: INVALID_CRC");
      //c_req_attempt_invalidcrc -= 1;
      result.is_err = true;
      return result;
    }
    
    result.is_err = false;
    result.len = req_len;
    result.crc8 = req_crc8;
    
    I2CSerialDebug.print(F("TinyWaitLen: "));
    I2CSerialDebug.println(req_len);
    I2CSerialDebug.print(F("TinyWaitCRC8: "));
    I2CSerialDebug.println(req_crc8, HEX);
    return result;
  }
  result.is_err = true;
  return result;
}

int8_t t_recv(const uint8_t wlen, const uint8_t wcrc8, uint8_t (*recv_array)[], uint8_t *recv_array_crc) {
  uint8_t c_req_attempt_req = 4;
  uint8_t c_req_attempt_wavailable = 4;

  while (c_req_attempt_req != 0) {
    const int8_t req_err = Wire.requestFrom(I2C_ADDR, (uint8_t)wlen); 
    yield();
    if (req_err< (int8_t)wlen) {
      I2CSerialDebug.print("InvalidReq3: ");
      I2CSerialDebug.println(req_err);
      delay(1);
      yield();
      c_req_attempt_req -= 1;
      
      continue;
    }
    if (Wire.available() < wlen) {
      uint8_t cc_req_attempt_wavailable = c_req_attempt_wavailable;
      while (cc_req_attempt_wavailable != 0) {
        delay(1);
        yield();
        
        if (Wire.available() < wlen) {
          cc_req_attempt_wavailable -= 1;
          continue;
        }else {
          break;
        }
      }
      if (cc_req_attempt_wavailable == 0) {
        return -2;
      }
    }
    

    uint8_t i = 0;
    uint8_t len = wlen;
    uint8_t out_crc =  0;
    I2CSerialDebug.print(F("ARRAY: "));
    while (len > i) {
      const uint8_t a = Wire.read();
      I2CSerialDebug.print(F("0x"));
      I2CSerialDebug.print(a, HEX);
      I2CSerialDebug.print(F(" "));
      yield();
      (*recv_array)[i++] = a;
      make_onesymb_crc8(out_crc, a);
    }
    I2CSerialDebug.println();
    I2CSerialDebug.print(F("CRC: "));
    I2CSerialDebug.println(out_crc, HEX);
    I2CSerialDebug.println();
    *recv_array_crc = out_crc;
    
    return (uint8_t)wlen;
  }

  return -1;
}

const int8_t send_and_recv(const uint8_t cmd, const uint8_t (*send_array)[] = NULL, const uint8_t send_len = 0, uint8_t (*recv_array)[] = NULL, const uint8_t recv_max = 0) {
  uint8_t c_send_attempt = 2;
  bool is_add_flushwti = false;
  while (c_send_attempt > 0) {
    if (!t_send(cmd, send_array, send_len)) {
      c_send_attempt -= 1;
      continue;
    }
    yield();
    delay(4); // wait tinythread..

    TRecvLenCrcResult c_lencrc8 = t_recv_lencrc();
    if (c_lencrc8.is_err) {
      return -3;
    }
    if (c_lencrc8.len != recv_max) {
      Serial.print(F("#exp_len: "));
      Serial.print(c_lencrc8.len);
      Serial.print(F(", max_len: "));
      Serial.print(recv_max);
      Serial.println(F(":"));
      Serial.println("ATTENTION, protocol violation, more expected data, further errors are possible. Adding automatic buffer clearing.");
      c_lencrc8.len = recv_max;

      while (Wire.available() > 0) {
        Wire.read();
      }
      return -4;
    }
    if (c_lencrc8.len == 0) {
      return 0;
    }
    uint8_t req_crc = 0;
    const int8_t req_len = t_recv(c_lencrc8.len, c_lencrc8.crc8, recv_array, &req_crc);
    if (req_len < 0) {
      I2CSerialDebug.print(F("t_recv, err: "));
      I2CSerialDebug.println(req_len);

      if (is_add_flushwti) {
        delay(10);
        while (Wire.available() > 0) {
          Wire.read();
        }
      }
      
      return req_len - 20;
    }
    if (c_lencrc8.crc8 != req_crc) {
      I2CSerialDebug.println(F("INVALID CRC8:("));
      I2CSerialDebug.println(req_crc);
      I2CSerialDebug.println(c_lencrc8.crc8);

      if (is_add_flushwti) {
        delay(10);
        while (Wire.available() > 0) {
          Wire.read();
        }
      }
      return -2;
    }

    if (is_add_flushwti) {
      delay(10);
      while (Wire.available() > 0) {
        Wire.read();
      }
    }
    return req_len;
  }
  I2CSerialDebug.println(F("send_and_recv: missed"));
  if (is_add_flushwti) {
    delay(10);
    while (Wire.available() > 0) {
      Wire.read();
    }
  }
  return -4;
}

void light_sleep() {
  if (digitalRead(D7) != HIGH) {
    Serial.println(F("Sleep now"));
    Serial.flush();
    
    wifi_station_disconnect();
    wifi_set_opmode(NULL_MODE);
    wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
    wifi_fpm_open();
    delay(100);
    gpio_pin_wakeup_enable(GPIO_ID_PIN(D7), GPIO_PIN_INTR_HILEVEL);
    wifi_fpm_do_sleep(0xFFFFFFF);
    delay(100);
    
    wifi_set_sleep_type(NONE_SLEEP_T);
    gpio_pin_wakeup_disable();
    wifi_fpm_close();
    wifi_set_opmode(STATION_MODE);
    wifi_station_connect();
    Serial.println(F("Woke up from sleep"));
  }else {
    Serial.println(F("SkipSleep now"));
    Serial.flush();
  }
}

unsigned long rtc_millis() {
  uint32_t now = system_get_time();
  unsigned long milliseconds = now / 1000;
  return milliseconds;
}

void setup() {
  Wire.begin();    
  //Wire.setClock(100000L); // Set speed at 100kHz
  //Wire.setClockStretchLimit(15000);
  Serial.begin(115200);
  pinMode(D7, INPUT); 
}

const uint8_t t_get_onemem(bool *is_err) {
  uint8_t recv_arr[] = {0};
  int8_t result = send_and_recv(
    I2C_CMD_USERONEMEM_GET, 
    NULL, 0, 
    &recv_arr, sizeof(recv_arr)
  );
  if ((int8_t)sizeof(recv_arr) > result) {
    *is_err = true;
  }else {
    *is_err = false; /* for ease of use */
  }
  
  return recv_arr[0];
}

void t_set_onemem(bool *is_err, const uint8_t a) {
  uint8_t recv_arr[] = {0};
  const uint8_t send_arr[] = {
    a
  };
  
  int8_t result = send_and_recv(
    I2C_CMD_USERONEMEM_SET, 
    &send_arr, sizeof(send_arr), 
    &recv_arr, sizeof(recv_arr)
  );
  if ((int8_t)sizeof(recv_arr) > result || recv_arr[0] != I2C_CMD_RESULT_OK) {
    *is_err = true;
  }else {
    *is_err = false; /* for ease of use */
  }
}

TFirmInfo t_firmware_info(bool *is_err) {
  TFirmInfo t_result;
  uint8_t recv_arr[] = {0, 0, 0};

  int8_t result = send_and_recv(
    I2C_CMD_INF, 
    NULL, 0, 
    &recv_arr, sizeof(recv_arr)
  );
  
  if ((int8_t)sizeof(recv_arr) > result) { /*0 - invalid, 4/5/6 - invalid format*/
    *is_err = true;
  }else {
    *is_err = false; /* for ease of use */
    t_result.FIRMWARE_VER = recv_arr[0];
    t_result.I2C_SIZE_RECVBUFF = recv_arr[1];
    t_result.I2C_SIZE_SENDBUFF = recv_arr[2];
  }
  
  return t_result;
}

TTimeInfo t_timer_info(bool *is_err) {
  TTimeInfo t_result;
  uint8_t recv_arr[9] = {0};

  int8_t result = send_and_recv(
    I2C_CMD_TIME_INF, 
    NULL, 0, 
    &recv_arr, sizeof(recv_arr)
  );
  if ((int8_t)sizeof(recv_arr) > result) { /*0 - invalid, 4/5/6 - invalid format*/
    *is_err = true;
  }else {
    *is_err = false; /* for ease of use */
    
    switch (recv_arr[0]) {
      case F_CPU_128KHZ:
        t_result.f_cpu = 128000;
        break;
      case F_CPU_600KHZ:
        t_result.f_cpu = 600000;
        break;
      case F_CPU_1_2MHZ:
        t_result.f_cpu = 1200000;
        break;
      case F_CPU_4_8MHZ:
        t_result.f_cpu = 4800000;
        break;
      case F_CPU_9_6MHZ:
        t_result.f_cpu = 9600000;
        break;
      default:
        t_result.f_cpu = 0;
        break;
    }
    switch((uint8_t)recv_arr[1]) {
      case (uint8_t)TTimerDivider::P0:
        t_result.divider = 0;
        break;
      case (uint8_t)TTimerDivider::P1:
        t_result.divider = 1;
        break;
      case (uint8_t)TTimerDivider::P8:
        t_result.divider = 8;
        break;
      case (uint8_t)TTimerDivider::P64:
        t_result.divider = 64;
        break;
      case (uint8_t)TTimerDivider::P256:
        t_result.divider = 256;
        break;
      case (uint8_t)TTimerDivider::P1024:
        t_result.divider = 1024;
        break;
      default:
        t_result.divider = 0;
        break;
    }

    t_result.current = recv_arr[2];
    t_result.ifnextevent = recv_arr[3];
    
    t_result.pin_0 = recv_arr[4];
    t_result.t0_mil = (recv_arr[5] << 8) | recv_arr[6];
    t_result.t0_event_mil = (recv_arr[7] << 8) | recv_arr[8];
  }
  
  return t_result;
}

void t_timer_setandreset(bool *is_err, const TTimerDivider divider, const uint8_t ocr0a, const uint16_t t0_event_mil) {
  const uint8_t send_arr[] = {
    t0_event_mil  & 0xFF,
    (t0_event_mil >> 8) & 0xFF,
    
    ocr0a,
    (uint8_t)divider
  };
  
  uint8_t recv_arr[1] = {0};
  int8_t result = send_and_recv(
    I2C_CMD_TIME_SETANDRESET,  
    &send_arr, sizeof(send_arr), 
    &recv_arr, sizeof(recv_arr)
  );
  if ((int8_t)sizeof(recv_arr) > result || recv_arr[0] != I2C_CMD_RESULT_OK) {
    *is_err = true;
  }else {
    *is_err = false; /* for ease of use */
  }
}

void t_timer_reset(bool *is_err) {
  uint8_t recv_arr[] = {0};

  int8_t result = send_and_recv(
    I2C_CMD_TIME_RESET,  
    NULL, 0, 
    &recv_arr, sizeof(recv_arr)
  );
  if ((int8_t)sizeof(recv_arr) > result || recv_arr[0] != I2C_CMD_RESULT_OK) {
    *is_err = true;
  }else {
    *is_err = false; /* for ease of use */
  }
}

void t_timer_trig0(bool *is_err) {
  uint8_t recv_arr[] = {0};

  int8_t result = send_and_recv(
    I2C_CMD_TIME_TRIG_PIN0,  
    NULL, 0, 
    &recv_arr, sizeof(recv_arr)
  );
  if ((int8_t)sizeof(recv_arr) > result || recv_arr[0] != I2C_CMD_RESULT_OK) {
    *is_err = true;
  }else {
    *is_err = false; /* for ease of use */
  }
}

void loop() {
  while (true) {
    I2CSerialDebug.println();
    I2CSerialDebug.println();

    bool is_err = false;
    /*Serial.println(F("#FIRMWARE:"));
    TFirmInfo t_firmware_result = t_firmware_info(&is_err);
    if (!is_err) {
      Serial.println(F("[===t_info===]"));
      Serial.print(F("FIRMWARE_VER: 0x"));
      Serial.println(t_firmware_result.FIRMWARE_VER, HEX);
      Serial.print(F("I2C_SIZE_RECVBUFF: "));
      Serial.println(t_firmware_result.I2C_SIZE_RECVBUFF);
      Serial.print(F("I2C_SIZE_SENDBUFF: "));
      Serial.println(t_firmware_result.I2C_SIZE_SENDBUFF);
    }
    Serial.println();*/
    Serial.println(F("#TIMER_SETANDRESET:"));
    t_timer_setandreset(&is_err, TTimerDivider::P1024, 255, 65535);
    if (!is_err) {
      Serial.println(F("TIMER_SETANDRESETOK"));
    }
    
    /*Serial.println(F("#TIMER_RESET:"));
    t_timer_reset(&is_err);
    if (!is_err) {
      Serial.println(F("OK"));
    }*/
    /*Serial.println(F("#TIMER_NEXTTRIG:"));
    t_timer_trig0(&is_err);
    if (!is_err) {
      Serial.println(F("OK"));
    }*/

    //while (true) {
    //t_timer_reset(&is_err);
    //delay(1000);
    Serial.println(F("#TIMER_INFO:"));
    TTimeInfo t_time_result = t_timer_info(&is_err);
    if (!is_err) {
      Serial.println(F("[===t_time===]"));
      Serial.print(F("f_cpu: "));
      Serial.println(t_time_result.f_cpu);
      Serial.print(F("divider: "));
      Serial.println(t_time_result.divider);
      Serial.print(F("current: "));
      Serial.println(t_time_result.current);
      Serial.print(F("ifnextevent: "));
      Serial.println(t_time_result.ifnextevent);
      Serial.println();
      Serial.print(F("pin_0: 0x"));
      Serial.println(t_time_result.pin_0, HEX);
      Serial.print(F("t0_mil: "));
      Serial.println(t_time_result.t0_mil);
      Serial.print(F("t0_event_mil: "));
      Serial.println(t_time_result.t0_event_mil);
      Serial.println();
    }
    Serial.println();
    if (!is_err) {
      light_sleep();
    }
    
    
    /*Serial.println(F("#USERMEMSET:"));
    const uint8_t newonemem = rand();
    t_set_onemem(&is_err, newonemem);
    Serial.print(F("newonemem: 0x"));
    Serial.println(newonemem, HEX);
    if (!is_err) {
      Serial.println(F("USERMEMSETOK"));
    }
    Serial.println();

    Serial.println(F("#USERMEMGET:"));
    uint8_t onemem = t_get_onemem(&is_err);
    if (!is_err) {
      Serial.print(F("ONEMEM: 0x"));
      Serial.println(onemem, HEX);
    }
    Serial.println();*/

    
    if (!is_err) {
      /*delay(2000);
      Serial.println(F("F:"));
      Serial.println(digitalRead(D7));*/
      
      /*while (digitalRead(D7) == LOW) {
        yield();
      }
      Serial.println(F("F:"));
      Serial.println(digitalRead(D7));*/
    }
    delay(1000);
    yield();
  }
}
