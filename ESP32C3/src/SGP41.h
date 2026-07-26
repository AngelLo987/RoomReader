#pragma once 
#include <Arduino.h>

struct SGP41data{ 
    //both are indexes that range from 0 to 500
    uint16_t no2Index; 
    uint16_t vocIndex; 
};

void sgpInit();
boolean sgpRead(SGP41data &data, float latestTemp, float latestHumidty); 