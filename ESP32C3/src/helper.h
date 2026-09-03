#pragma once

#include <vector>
#include <time.h>

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
);
