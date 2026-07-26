#include <Arduino.h>
#include "pms5003.h"
#include "SCD40.h"
#include "SGP41.h"
#include "secrets.h"
#include <ezTime.h>
#include <WiFi.h>

//for the SGP41 to input and get readings 
float latestTemp = 25.0; 
float latestHumidty = 50.0; 

//to make sure that the ESP32 calls on it properly for readings 
Timezone myTZ;
time_t lastReadSCD40 = myTZ.now();
time_t lastReadSGP41 = myTZ.now(); 
//Wifi and ezTime data
const char* location = "America/Los_Angeles";




void setup() {
  //start serial
  Serial.begin(115200);
  delay(200);
  Serial.println("Serial connected");

  //connect to wifi  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
  }
  Serial.println("connected to wifi");

  //Initializing Timezone 
  waitForSync();
  myTZ.setLocation(F(location));
  myTZ.setDefault();

  //Initialize all of the sernsors
  pmsInit(); //PMS5003 
  scd40Init(); //SCD40 sensor 
  sgpInit(); //SGP41 sensor
  delay(1000);
  Serial.println("Ready to Read");
}

void loop() {
  //Reading PMS5003 data
  PMSdata pmsData; 
  if (pmsRead(pmsData)){ 
    Serial.print("PM1.0 ");
    Serial.println(pmsData.pm1_0);
    Serial.print("PM2.5: ");
    Serial.println(pmsData.pm2_5);
    Serial.print("PM10.0: "); 
    Serial.println(pmsData.pm10_0);
  }

  //Reading SCD40 data which is every 5 seconds (NEST INSIDE OF SGP41)
  SCD40data scdData;
  if (myTZ.now() - lastReadSCD40 >= 5){
    if (scd40Read(scdData)){
      Serial.print("CO2: "); 
      Serial.println(scdData.co2);
      Serial.print("Humidty: ");
      Serial.println(scdData.humidity);
      Serial.print("Temperature: ");
      Serial.println(scdData.temperature);
      lastReadSCD40 = myTZ.now(); 
      latestHumidty = scdData.humidity; 
      latestTemp = scdData.temperature;
    }

  }
  //Reading SGP41 data which is every 1 second 
  SGP41data sgpdata;
  if (myTZ.now() - lastReadSGP41 >= 1){
    if(sgpRead(sgpdata, latestTemp, latestHumidty)){
      Serial.print("Nox Index: ");
      Serial.println(sgpdata.no2Index);
      Serial.print("Voc Index: ");
      Serial.println(sgpdata.vocIndex);
      lastReadSGP41 = myTZ.now(); 
    }
  }
}
