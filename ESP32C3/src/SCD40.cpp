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
    
    uint16_t humidity_raw = combineBytes(frame.data(), HumidIndex);
    uint16_t temp_raw= combineBytes(frame.data(), TempIndex);

    
    data.co2 = combineBytes(frame.data(), CO2Index);
    data.temperature = tempConvert(temp_raw);
    data.humidity = humidityConvert(humidity_raw); 

    return true; 
}






