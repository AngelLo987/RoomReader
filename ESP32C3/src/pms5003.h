#pragma once
#include <Arduino.h>

struct PMSdata{
    uint16_t pm1_0; //concetration of pms1.0 particles
    uint16_t pm2_5; //concentration of pms 2.5 particles
    uint16_t pm10_0; //concentraton of pms 10 particles
};

void pmsInit();

bool pmsRead(PMSdata &data);





