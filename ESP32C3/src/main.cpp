#include <Arduino.h>
#include "pms5003.h"
#include "SCD40.h"
#include "SGP41.h"
#include "secrets.h"
#include <ezTime.h>
#include <WiFi.h>
#include <vector>
#include "helper.h"

//Initiate time object
Timezone myTZ;

//Arrays needed to keep track of values to average over 120 seconds
int uploadInterval = 120; //120 seconds
  //PMS5003//
std::vector<uint16_t> PMS1_0Data;
std::vector<uint16_t> PMS2_5Data;
std::vector<uint16_t> PMS10_0Data;
  //SGP41//
std::vector<float> noxData;
std::vector<float> vocData;
  //SCD40//
std::vector<uint16_t> co2Data;
std::vector<float> tempData;
std::vector<float> humidData;
  //Keep track of time
time_t lastAveraged = myTZ.now();


//for the SGP41 to input and get readings
float latestTemp = 25.0;
float latestHumidty = 50.0;

//to make sure that the ESP32 calls on sensors properly for readings
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
    //put data into arrays
    PMS1_0Data.push_back(pmsData.pm1_0);
    PMS2_5Data.push_back(pmsData.pm2_5);
    PMS10_0Data.push_back(pmsData.pm10_0);
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
      //Add into arrays
      co2Data.push_back(scdData.co2);
      tempData.push_back(scdData.temperature);
      humidData.push_back(scdData.humidity);
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
      //add into arrays
      noxData.push_back(sgpdata.no2Index);
      vocData.push_back(sgpdata.vocIndex);
    }
  }

  //check if 120 seconds have passed before averaging and sendinf to server
  if (myTZ.now() - lastAveraged >= uploadInterval){
      //make sure to put in the elements into your lists as you go
      //average and CLEAR the elements as well
      //reset the lastAveraged variable
      // SEND THE PACKET to server

      double avgPMS1_0 = getAverage(PMS1_0Data);
      double avgPMS2_5 = getAverage(PMS2_5Data);
      double avgPMS10_0 = getAverage(PMS10_0Data);

      double avgNOX = getAverage(noxData);
      double avgVOC = getAverage(vocData);

      double avgCO2 = getAverage(co2Data);
      double avgTemp = getAverage(tempData);
      double avgHumid = getAverage(humidData);

      int code = sendReading(deviceid, avgPMS1_0, avgPMS2_5,avgPMS10_0,avgNOX,avgVOC,avgCO2,avgTemp, avgHumid, "/readings");
      if (code == 201){
        Serial.write("120s reading sent successfully");
        lastAveraged = myTZ.now();
        PMS1_0Data.clear();
        PMS2_5Data.clear();
        PMS10_0Data.clear();
        noxData.clear();
        vocData.clear();
        co2Data.clear();
        tempData.clear();
        humidData.clear();
      }
      else{
        Serial.write("120s reading set unsuccessfully");
      }


  }

}
