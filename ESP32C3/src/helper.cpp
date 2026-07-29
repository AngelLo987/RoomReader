#include "helper.h"
#include "secrets.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include <cmath>

int sendReading(
    const char* device_id,
    double avgPMS1_0,
    double avgPMS2_5,
    double avgPMS10_0,
    double avgNOX,
    double avgVOC,
    double avgCO2,
    double avgTemp,
    double avgHumid,
    const char* endpoint
) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reading upload skipped: Wi-Fi is disconnected");
        return -1;
    }

    String fullURL = String(baseURL) + endpoint;

    JsonDocument json;
    json["device_id"] = device_id;
    json["pm1_0"] = avgPMS1_0;
    json["pm2_5"] = avgPMS2_5;
    json["pm10_0"] = avgPMS10_0;
    json["co2"] = std::lround(avgCO2);
    json["temp_c"] = avgTemp;
    json["rh_percent"] = avgHumid;
    json["voc_index"] = std::lround(avgVOC);
    json["nox_index"] = std::lround(avgNOX);

    String payload;
    serializeJson(json, payload);

    HTTPClient http;
    http.setTimeout(5000);

    if (!http.begin(fullURL)) {
        Serial.println("Unable to initialize HTTP connection");
        return -1;
    }

    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
        Serial.printf("HTTP status: %d\n", httpResponseCode);
        Serial.println(http.getString());
    } else {
        Serial.printf(
            "HTTP request failed: %s\n",
            HTTPClient::errorToString(httpResponseCode).c_str()
        );
    }

    http.end();
    return httpResponseCode;
}
