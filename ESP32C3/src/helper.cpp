#include "helper.h"
#include "secrets.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>

#include <cmath>

constexpr char firmwareVersion[] = "0.1.0";

int sendReading(const Reading& reading, const char* endpoint) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reading upload skipped: Wi-Fi is disconnected");
        return -1;
    }

    String fullURL = String(baseURL) + endpoint;
    struct tm utcTime;
    gmtime_r(&reading.recordedAt, &utcTime);

    char recordedAtText[25];

    strftime(
        recordedAtText,
        sizeof(recordedAtText),
        "%Y-%m-%dT%H:%M:%SZ",
        &utcTime
    );

    JsonDocument json;
    json["device_id"] = reading.deviceId;
    json["recorded_at"] = recordedAtText;

    if (reading.pm1_0.available) {
        json["pm1_0"] = reading.pm1_0.value;
    } else {
        json["pm1_0"] = nullptr;
    }
    if (reading.pm2_5.available) {
        json["pm2_5"] = reading.pm2_5.value;
    } else {
        json["pm2_5"] = nullptr;
    }
    if (reading.pm10_0.available) {
        json["pm10_0"] = reading.pm10_0.value;
    } else {
        json["pm10_0"] = nullptr;
    }
    if (reading.co2.available) {
        json["co2"] = std::lround(reading.co2.value);
    } else {
        json["co2"] = nullptr;
    }
    if (reading.temperature.available) {
        json["temp_c"] = reading.temperature.value;
    } else {
        json["temp_c"] = nullptr;
    }
    if (reading.humidity.available) {
        json["rh_percent"] = reading.humidity.value;
    } else {
        json["rh_percent"] = nullptr;
    }
    if (reading.vocIndex.available) {
        json["voc_index"] = std::lround(reading.vocIndex.value);
    } else {
        json["voc_index"] = nullptr;
    }
    if (reading.noxIndex.available) {
        json["nox_index"] = std::lround(reading.noxIndex.value);
    } else {
        json["nox_index"] = nullptr;
    }

    json["wifi_rssi"] = reading.wifiRssi;
    json["uptime_seconds"] = reading.uptimeSeconds;
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
