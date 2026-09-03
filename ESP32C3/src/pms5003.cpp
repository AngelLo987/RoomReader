#include "pms5003.h"
#include <array>

const int PMS_RX_PIN = 4; //pin 4 on ESP32
const int PMS_TX_PIN = 3; //pin 3 on ESP32
const int PMS_BAUD = 9600; //UART speed for sensor
const int PMS_FRAME_SIZE = 32;

//frame indexes for the PMSdata struct
int PM1_0 = 10; //starts at 10
int PM2_5 = 12; //starts at 12
int PM10_0 = 14; //starts at 14


//Helper Functions//


/*
return merged number of two bytes. The index refers to the high byte

The order that frames are sent in is High then Low
*/
static uint16_t combineBytes(const uint8_t *frame, int index){
    uint8_t high = frame[index];
    //move the high byte 8 spaces to the left
    uint16_t combined = high << 8;
    //add the low byte without any formatting since first 8 spaces are free
    combined += frame[index+1];
    return combined;
}



HardwareSerial pmsSerial(1); //using UART 1 for sensor

void pmsInit(){
    pmsSerial.begin(PMS_BAUD, SERIAL_8N1, PMS_RX_PIN, PMS_TX_PIN);
}



bool pmsRead(PMSdata &data){
    //available function will look inside serial port's recieiving box and return exact number of bytes waiting to be read
    if (pmsSerial.available() < PMS_FRAME_SIZE){
        return false;
    }
    while (pmsSerial.peek() != 0x42 && pmsSerial.available() > 0){
        pmsSerial.read(); //remove the byte from the buffer
        if (pmsSerial.available() < PMS_FRAME_SIZE){
            return false;
        }
    }
    uint8_t byte1 = pmsSerial.read();
    //second byte needs to be equal to 0x4D
    if (pmsSerial.peek() != 0x4D){
        return false;
    }
    //frame is a array is 32 bytes long (32 uint8_t's)
    std::array<uint8_t, PMS_FRAME_SIZE> frame;
    frame[0] = byte1;
    frame[1] = pmsSerial.read();
    for (int i = 2; i < PMS_FRAME_SIZE; i++){
        frame[i] = pmsSerial.read();
    }

    //verify the frame length
    uint16_t frameLength = combineBytes(frame.data(), 2);
    if (frameLength != 0x001C) {
      Serial.println("PMS5003 invalid frame length");
      return false;
    }

    //checksum (from bytes 0 to 29) should equal the last two bytes (30 and 31)
    uint16_t sum = 0;
    for (int i = 0; i < PMS_FRAME_SIZE - 2; i++){
        sum += frame[i];
    }
    uint16_t expectedChecksum = (static_cast<uint16_t>(frame[30]) << 8) + frame[31];
    if (sum != expectedChecksum){
        return false;
    }

    //check error byte
    if (frame[29] != 0x00){
        //hardware failure
        return false;
    }




    //frame.data() gives the pointer to the frame array
    data.pm1_0 = combineBytes(frame.data(), PM1_0);
    data.pm2_5 = combineBytes(frame.data(), PM2_5);
    data.pm10_0 = combineBytes(frame.data(), PM10_0);
    return true;




}






