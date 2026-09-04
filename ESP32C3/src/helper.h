#pragma once

#include <vector>
#include <stdint.h>
#include <time.h>

struct ReadingValue {
    ReadingValue(double readingValue = 0.0, bool isAvailable = false)
        : value(readingValue), available(isAvailable) {}

    double value;
    bool available;
};

// One complete two-minute reading. Keeping the data together makes readings
// easy to pass to the uploader and retain for a later retry.
struct Reading {
    const char* deviceId = nullptr;
    time_t recordedAt = 0;
    ReadingValue pm1_0;
    ReadingValue pm2_5;
    ReadingValue pm10_0;
    ReadingValue noxIndex;
    ReadingValue vocIndex;
    ReadingValue co2;
    ReadingValue temperature;
    ReadingValue humidity;
    int32_t wifiRssi = 0;
    uint32_t uptimeSeconds = 0;
};

template <typename T>
bool getAverage(const std::vector<T>& values, double& average) {
    if (values.empty()) {
        return false;
    }

    double total = 0.0;

    for (const T& value : values) {
        total += static_cast<double>(value);
    }

    average = total / values.size();
    return true;
}

int sendReading(const Reading& reading, const char* endpoint);
