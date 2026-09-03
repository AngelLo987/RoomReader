#include "helper.h"
#include "secrets.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include <cmath>

constexpr char firmwareVersion[] = "0.1.0";

int sendReading(
    const char* device_id,
    time_t recorded_at,
    double avgPMS1_0,
    bool hasPMS1_0,
    double avgPMS2_5,
    bool hasPMS2_5,
    double avgPMS10_0,
    bool hasPMS10_0,
    double avgNOX,
    bool hasNOX,
    double avgVOC,
    bool hasVOC,
    double avgCO2,
    bool hasCO2,
    double avgTemp,
    bool hasTemp,
    double avgHumid,
    bool hasHumid,
    const char* endpoint
) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reading upload skipped: Wi-Fi is disconnected");
        return -1;
    }

    String fullURL = String(baseURL) + endpoint;
    struct tm utcTime;
    gmtime_r(&recorded_at, &utcTime);

    char recordedAtText[25];

    strftime(
        recordedAtText,
        sizeof(recordedAtText),
        "%Y-%m-%dT%H:%M:%SZ",
        &utcTime
    );

    JsonDocument json;
    json["device_id"] = device_id;
    json["recorded_at"] = recordedAtText;

    if (hasPMS1_0) {
        json["pm1_0"] = avgPMS1_0;
    } else {
        json["pm1_0"] = nullptr;
    }
    if (hasPMS2_5) {
        json["pm2_5"] = avgPMS2_5;
    } else {
        json["pm2_5"] = nullptr;
    }
    if (hasPMS10_0) {
        json["pm10_0"] = avgPMS10_0;
    } else {
        json["pm10_0"] = nullptr;
    }
    if (hasCO2) {
        json["co2"] = std::lround(avgCO2);
    } else {
        json["co2"] = nullptr;
    }
    if (hasTemp) {
        json["temp_c"] = avgTemp;
    } else {
        json["temp_c"] = nullptr;
    }
    if (hasHumid) {
        json["rh_percent"] = avgHumid;
    } else {
        json["rh_percent"] = nullptr;
    }
    if (hasVOC) {
        json["voc_index"] = std::lround(avgVOC);
    } else {
        json["voc_index"] = nullptr;
    }
    if (hasNOX) {
        json["nox_index"] = std::lround(avgNOX);
    } else {
        json["nox_index"] = nullptr;
    }

    json["wifi_rssi"] = WiFi.RSSI();
    json["uptime_seconds"] = millis() / 1000;
    json["firmware_version"] = firmwareVersion;

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
