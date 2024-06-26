
#include <Wire.h>
 
void setup(){
    Wire.begin();    
    Wire.setClock(100000L); // Set speed at 100kHz
    Wire.setClockStretchLimit(15000);
    Serial.begin(115200);
    
    while (!Serial);
    Serial.println("\nI2C Scanner");
    while (Serial.available() > 0) {
      Serial.read();
      yield();
    }
} 
 
void loop(){
    while (Serial.available() == 0) {
      yield();
    }
    byte error, address;
    int nDevices;
 
    Serial.println("Scanning...");
 
    nDevices = 0;
    for(address = 0; address < 127; address++ ){
      //if (address == 0x61) continue;
      yield();
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
 
        if (error == 0){
            Serial.print("I2C device found at address 0x");
            if (address<16)
                Serial.print("0");
            Serial.print(address,HEX);
            Serial.println(" !");

            uint8_t size = (uint8_t)Serial.available();
            /*if (size > 10) {
              size = 9;
            }*/

            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read();
            }
            
            /*Serial.print("Recv: ");
            Wire.requestFrom(address, (uint8_t)size);    // request 6 bytes from slave device #8
            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read(); // receive a byte as character
              Serial.print(c, HEX);         // print the character
              Serial.print(" ");
              yield();
            }
            Serial.println();*/

            /*while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read();
            }
            
            Serial.print("Recv: ");
            Wire.requestFrom(address, (uint8_t)20);    // request 6 bytes from slave device #8
            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read(); // receive a byte as character
              Serial.print(c, HEX);         // print the character
              Serial.print(" ");
              yield();
            }*/

            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read();
            }
           if (size > 0) {
              Serial.println();
              Serial.print("Send..");
              Wire.beginTransmission(address);
              uint8_t size2 = size;
              while (size2 > 0) {
                uint8_t read = (uint8_t)Serial.read();
                Wire.write(read);
                Serial.print(read, HEX);         // print the character
                Serial.print(" ");
                size2 -= 1;            

              }
              uint8_t err2 = Wire.endTransmission();
              Serial.println();
              Serial.print("ERR: ");
              Serial.println(err2, HEX);
            }
            yield();
            
            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read();
            }

            Serial.println();
            Serial.print("Recv: ");
            Wire.requestFrom(address, (uint8_t)size);    // request 6 bytes from slave device #8
            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read(); // receive a byte as character
              Serial.print(c, HEX);         // print the character
              Serial.print(" ");
              yield();
            }

            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read();
            }
            
            Serial.println();
            Serial.print("Recv: ");
            Wire.requestFrom(address, (uint8_t)20);    // request 6 bytes from slave device #8
            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read(); // receive a byte as character
              Serial.print(c, HEX);         // print the character
              Serial.print(" ");
              yield();
            }

            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read();
            }
            
            Serial.println();
            Serial.print("Recv: ");
            Wire.requestFrom(address, (uint8_t)size);    // request 6 bytes from slave device #8
            while (Wire.available() > 0) { // slave may send less than requested
              uint8_t c = Wire.read(); // receive a byte as character
              Serial.print(c, HEX);         // print the character
              Serial.print(" ");
            }

            /*if (size > 0) {
              Serial.println();
              Serial.print("Send..");
              Wire.beginTransmission(address);
              uint8_t size2 = size/2;
              while (size2 > 0) {
                uint8_t read = (uint8_t)Serial.read();
                Wire.write(read);
                Serial.print(read, HEX);         // print the character
                Serial.print(" ");
                size2 -= 1;
              }
              uint8_t err2 = Wire.endTransmission();
              Serial.println();
              Serial.print("ERR: ");
              Serial.println(err2, HEX);
            }
            */
            nDevices++;
        }
        else if (error==4) {
            Serial.print("Unknow error at address 0x");
            if (address<16)
                Serial.print("0");
            Serial.println(address,HEX);
        } 
    }
    if (nDevices == 0)
        Serial.println("No I2C devices found\n");
    else
        Serial.println("done\n");
 
    //delay(500);
    
    //Serial.read();
    while (Serial.available() > 0) {
      Serial.read();
      yield();
    }
}
