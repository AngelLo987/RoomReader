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
    uint8_t byte1 = pmsSerial.read();
    //first byte needs to be equal to 0x42
    if (byte1 != 0x42){
        while (pmsSerial.available() > 0){
            pmsSerial.read();
        }
        return false;
    }
    uint8_t byte2 = pmsSerial.peek();
    //second byte needs to be equal to 0x4D
    if (byte2 != 0x4D){
        while (pmsSerial.available() > 0){
            pmsSerial.read();
        }
        return false;
    }
    //frame is a array is 32 bytes long (32 uint8_t's)
    std::array<uint8_t, PMS_FRAME_SIZE> frame;
    frame[0] = byte1;
    frame[1] = pmsSerial.read();
    for (int i = 2; i < PMS_FRAME_SIZE; i++){
        frame[i] = pmsSerial.read();
    }

    //frame.data() gives the pointer to the frame array
    data.pm1_0 = combineBytes(frame.data(), PM1_0);
    data.pm2_5 = combineBytes(frame.data(), PM2_5);
    data.pm10_0 = combineBytes(frame.data(), PM10_0);
    return true;

}






