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
constexpr int uploadInterval = 120; // 120 seconds
constexpr size_t maxPendingReadings = 30; // One hour at the current interval.
Reading pendingReadings[maxPendingReadings];
size_t pendingReadingsStart = 0;
size_t pendingReadingsCount = 0;
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
  //Initialized in setup() after the ESP32 clock has synchronized.
time_t lastAveraged;


//for the SGP41 to input and get readings
float latestTemp = 25.0;
float latestHumidity = 50.0;

//to make sure that the ESP32 calls on sensors properly for readings
time_t lastReadSCD40;
time_t lastReadSGP41;
//Wifi and ezTime data
constexpr char location[] = "America/Los_Angeles";

void queueReading(const Reading& reading) {
  if (pendingReadingsCount == maxPendingReadings) {
    Serial.println("Pending queue full; discarding oldest reading");
    pendingReadingsStart = (pendingReadingsStart + 1) % maxPendingReadings;
    pendingReadingsCount--;
  }

  size_t nextIndex =
    (pendingReadingsStart + pendingReadingsCount) % maxPendingReadings;
  pendingReadings[nextIndex] = reading;
  pendingReadingsCount++;
}

void sendPendingReadings() {
  while (pendingReadingsCount > 0) {
    int code = sendReading(pendingReadings[pendingReadingsStart], "/readings");
    if (code != 201) {
      Serial.printf(
        "Upload failed; %u reading(s) waiting for retry\n",
        static_cast<unsigned int>(pendingReadingsCount)
      );
      return;
    }

    pendingReadingsStart = (pendingReadingsStart + 1) % maxPendingReadings;
    pendingReadingsCount--;
    Serial.println("Queued reading sent successfully");
  }
}




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
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Setting clock");
  while (time(nullptr) < 1700000000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  myTZ.setLocation(location);
  myTZ.setDefault();

  lastAveraged = myTZ.now();
  lastReadSCD40 = myTZ.now();
  lastReadSGP41 = myTZ.now();

  Serial.println("Clock set");

  // Initialize all of the sensors
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
    Serial.print("PM1.0: ");
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
  else{
    Serial.println("PMS5003 read failed");
  }

  //Reading SCD40 data which is every 5 seconds (NEST INSIDE OF SGP41)
  SCD40data scdData;
  if (myTZ.now() - lastReadSCD40 >= 5){
    // Advance the schedule even when a read fails so a disconnected sensor
    // does not cause the loop to hammer the I2C bus continuously.
    lastReadSCD40 = myTZ.now();
    if (scd40Read(scdData)){
      Serial.print("CO2: ");
      Serial.println(scdData.co2);
      Serial.print("Humidty: ");
      Serial.println(scdData.humidity);
      Serial.print("Temperature: ");
      Serial.println(scdData.temperature);
      latestHumidity = scdData.humidity;
      latestTemp = scdData.temperature;
      //Add into arrays
      co2Data.push_back(scdData.co2);
      tempData.push_back(scdData.temperature);
      humidData.push_back(scdData.humidity);
    }
    else{
      Serial.println("SCD40 read failed");
    }

  }
  //Reading SGP41 data which is every 1 second
  SGP41data sgpdata;
  if (myTZ.now() - lastReadSGP41 >= 1){
    // The gas index algorithms expect a 1 Hz cadence. Avoid rapid retries on
    // failures, which would otherwise feed the algorithm too frequently.
    lastReadSGP41 = myTZ.now();
    if(sgpRead(sgpdata, latestTemp, latestHumidity)){
      Serial.print("Nox Index: ");
      Serial.println(sgpdata.noxIndex);
      Serial.print("Voc Index: ");
      Serial.println(sgpdata.vocIndex);
      //add into arrays
      noxData.push_back(sgpdata.noxIndex);
      vocData.push_back(sgpdata.vocIndex);
    }
    else {
      Serial.println("SGP41 read failed");
    }
  }

  // Check if 120 seconds have passed before averaging and sending to server.
  if (myTZ.now() - lastAveraged >= uploadInterval){
      //make sure to put in the elements into your lists as you go
      //average and CLEAR the elements as well
      //reset the lastAveraged variable
      // SEND THE PACKET to server

      double avgPMS1_0 = 0.0;
      double avgPMS2_5 = 0.0;
      double avgPMS10_0 = 0.0;
      bool hasPMS1_0 = getAverage(PMS1_0Data, avgPMS1_0);
      bool hasPMS2_5 = getAverage(PMS2_5Data, avgPMS2_5);
      bool hasPMS10_0 = getAverage(PMS10_0Data, avgPMS10_0);

      double avgNOX = 0.0;
      double avgVOC = 0.0;
      bool hasNOX = getAverage(noxData, avgNOX);
      bool hasVOC = getAverage(vocData, avgVOC);

      double avgCO2 = 0.0;
      double avgTemp = 0.0;
      double avgHumid = 0.0;
      bool hasCO2 = getAverage(co2Data, avgCO2);
      bool hasTemp = getAverage(tempData, avgTemp);
      bool hasHumid = getAverage(humidData, avgHumid);

      Reading reading;
      reading.deviceId = deviceid;
      reading.recordedAt = time(nullptr);
      reading.pm1_0 = {avgPMS1_0, hasPMS1_0};
      reading.pm2_5 = {avgPMS2_5, hasPMS2_5};
      reading.pm10_0 = {avgPMS10_0, hasPMS10_0};
      reading.noxIndex = {avgNOX, hasNOX};
      reading.vocIndex = {avgVOC, hasVOC};
      reading.co2 = {avgCO2, hasCO2};
      reading.temperature = {avgTemp, hasTemp};
      reading.humidity = {avgHumid, hasHumid};
      reading.wifiRssi = WiFi.RSSI();
      reading.uptimeSeconds = millis() / 1000;

      queueReading(reading);
      sendPendingReadings();

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

}
