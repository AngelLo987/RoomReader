#include "SGP41.h"
#include <array>
#include <Wire.h> //for I2C 
#include <VOCGasIndexAlgorithm.h>
#include <NOxGasIndexAlgorithm.h>

//Algorithm objects 
VOCGasIndexAlgorithm vocAlgorithm;
NOxGasIndexAlgorithm noxAlgorithm;

//gloabal variables pertainintg to sensor 
const int SCLpin = 6;
const int SDApin = 7;
constexpr size_t frameLength = 6; //6 byte frames 
const int I2Caddress = 0x59; 
constexpr bool debugSgp41 = false;

//frame indexes for SGP41 structs 
const int CO2Index = 0; 
const int TempIndex = 3;

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

// Standard Sensirion CRC-8 Algorithm
static uint8_t generateCRC(const uint8_t* data, uint16_t count) {
    uint8_t crc = 0xFF;
    for(uint16_t i = 0; i < count; i++) {
        crc ^= data[i];
        for(uint8_t bit = 8; bit > 0; --bit) {
            if(crc & 0x80) crc = (crc << 1) ^ 0x31;
            else crc = (crc << 1);
        }
    }
    return crc;
}

//convert tick to scale
int vocIndex(int raw){ 
    return vocAlgorithm.process(raw);
}
//convert tick to scale
int noxIndex(int raw){
    return noxAlgorithm.process(raw);
}

void sgpInit(){ 
    Wire.begin(SDApin, SCLpin);

    vocAlgorithm.reset();
    noxAlgorithm.reset();
}

boolean sgpRead(SGP41data &data, float latestTemp, float latestHumidty){
    uint16_t rh_ticks = (uint16_t)((latestHumidty*65535.0)/100.0); //percentage out of 16-bits
    uint16_t temp_ticks = (uint16_t)(((latestTemp + 45.0)*65535)/175.0); //45 offset since lowest possble readings is -45 which gives us a safe zero

    //split into high and low bytes
    std::array<uint8_t, 2> rh_bytes = {
        (uint8_t)(rh_ticks >> 8),
        (uint8_t)(rh_ticks & 0xFF)
    };
    std::array<uint8_t, 2> temp_bytes = {
        (uint8_t)(temp_ticks >> 8),
        (uint8_t)(temp_ticks & 0xFF)
    };

    Wire.beginTransmission(I2Caddress);
    //measure raw signals command 
    Wire.write(0x26); //high
    Wire.write(0x19); //low

    //Send humidity data +CRC
    Wire.write(rh_bytes[0]);
    Wire.write(rh_bytes[1]);
    Wire.write(generateCRC(rh_bytes.data(), 2));

    //Send temperature data + CRC 
    Wire.write(temp_bytes[0]);
    Wire.write(temp_bytes[1]);
    Wire.write(generateCRC(temp_bytes.data(), 2));

    uint8_t txStatus = Wire.endTransmission();
    if (txStatus != 0) {
        Serial.print("SGP41 I2C write failed. endTransmission status: ");
        Serial.println(txStatus);
        return false;
    }

    delay(50);

    //Get the 6 bytes back
    uint8_t bytesRead = Wire.requestFrom((uint8_t)I2Caddress, (uint8_t)frameLength);
    if (bytesRead != frameLength || Wire.available() < frameLength){
        Serial.print("SGP41 I2C read failed. bytesRead: ");
        Serial.print(bytesRead);
        Serial.print(" available: ");
        Serial.println(Wire.available());
        return false;
    }

    std::array<uint8_t, 6> rxData; 
    for (int i = 0; i < frameLength; i++){
        rxData[i] = Wire.read(); 
    }

    uint8_t vocCrc = generateCRC(rxData.data(), 2);
    uint8_t noxCrc = generateCRC(rxData.data() + 3, 2);
    if (vocCrc != rxData[2] || noxCrc != rxData[5]) {
        Serial.print("SGP41 CRC failed. VOC expected/read: ");
        Serial.print(vocCrc);
        Serial.print("/");
        Serial.print(rxData[2]);
        Serial.print(" NOx expected/read: ");
        Serial.print(noxCrc);
        Serial.print("/");
        Serial.println(rxData[5]);
        return false;
    }

    uint16_t srawVoc = combineBytes(rxData.data(), 0);
    uint16_t srawNox = combineBytes(rxData.data(), 3);

    data.srawVoc = srawVoc;
    data.srawNox = srawNox;
    data.vocIndex = vocIndex(srawVoc);
    data.no2Index = noxIndex(srawNox);

    if (debugSgp41) {
        Serial.print("SGP41 raw VOC: ");
        Serial.print(data.srawVoc);
        Serial.print(" raw NOx: ");
        Serial.print(data.srawNox);
        Serial.print(" VOC index: ");
        Serial.print(data.vocIndex);
        Serial.print(" NOx index: ");
        Serial.print(data.no2Index);
        Serial.print(" T/H comp: ");
        Serial.print(latestTemp);
        Serial.print("C ");
        Serial.print(latestHumidty);
        Serial.println("%");
    }

    return true;
}
