#pragma once
#include <Arduino.h>


struct SCD40data { 
    uint16_t co2; //ppm
    float temperature; //Celsius
    float humidity; //% RH
};

void scd40Init(); 

boolean scd40Read(SCD40data &data); 
