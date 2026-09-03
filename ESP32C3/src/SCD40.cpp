#include "SCD40.h"
#include <array>
#include <Wire.h> //for I2C

const int SCLpin = 6;
const int SDApin = 7;
constexpr size_t frameLength = 9; //9 byte frames
const int I2Caddress = 0x62;


//frame indexes for SCD40 structs
const int CO2Index = 0;
const int TempIndex = 3;
const int HumidIndex = 6;


//HELPER FUNCTIONS//

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

//Helper functions to convert raw ticks to their units

//Temperature in C*
float tempConvert(uint16_t raw){
    return ((175.0*raw)/65536)-45;
}
//Humidty %RH
float humidityConvert(uint16_t raw){
    return (100.0 * raw)/65536;
}

//Helper fucntion to calculate the CRC8 checksum of a given data array. The CRC8 polynomial used is 0x31 (x^8 + x^5 + x^4 + 1).
uint8_t calcCRC(uint8_t *data, size_t len) {
    uint8_t crc = 0xFF; // Initial value
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i]; // XOR byte into least sig. byte of crc
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31; // Polynomial
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}



void scd40Init(){
    //initialize I2C hardware connection bus
    Wire.begin(SDApin, SCLpin);

    //Open a connection to sensor
    Wire.beginTransmission(I2Caddress);



    //command to get SCD40 to start reading (0x21b1)
    Wire.write(0x21); //high
    Wire.write(0xB1); //low
    Wire.endTransmission();
}

bool scd40Read(SCD40data &data){
    //creating the 9 byte frame
    Wire.beginTransmission(I2Caddress);
    //The command (0xEC05) will get the Sensor to send the 9byte frame back to us
    Wire.write(0xEC);
    Wire.write(0x05);
    Wire.endTransmission();
    delay(1);

    //request 9 bytes from sensor
    Wire.requestFrom(I2Caddress, ((uint8_t)frameLength));
        if (Wire.available() < frameLength){
            return false;
        }
        std::array<uint8_t, frameLength> frame;
        for (int i = 0; i < frameLength; i++){
            frame[i] = Wire.read();
        }

    //cehcksum verification (there are 3)

    //CO2 cehcksum
    uint8_t checksum = calcCRC(&frame[0], 2);
    if (checksum != frame[2]){
        return false;
    }
    //Temperature checksum
    checksum = calcCRC(&frame[3], 2);
    if (checksum != frame[5]){
        return false;
    }
    //RH checksum
    checksum = calcCRC(&frame[6], 2);
    if (checksum != frame[8]){
        return false;
    }

    uint16_t humidity_raw = combineBytes(frame.data(), HumidIndex);
    uint16_t temp_raw= combineBytes(frame.data(), TempIndex);


    data.co2 = combineBytes(frame.data(), CO2Index);
    data.temperature = tempConvert(temp_raw);
    data.humidity = humidityConvert(humidity_raw);

    return true;
}






