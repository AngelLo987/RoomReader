#pragma once
#include <Arduino.h>

struct SGP41data{
    //raw sensor ticks before Sensirion gas index algorithm processing
    uint16_t srawVoc;
    uint16_t srawNox;

    //both are indexes that range from 0 to 500
    uint16_t noxIndex;
    uint16_t vocIndex;
};

void sgpInit();
boolean sgpRead(SGP41data &data, float latestTemp, float latestHumidty);
