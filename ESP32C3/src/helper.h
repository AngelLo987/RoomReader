#pragma once

#include <vector>

template <typename T>
double getAverage(const std::vector<T>& values) {
    if (values.empty()) {
        return 0.0;
    }

    double total = 0.0;

    for (const T& value : values) {
        total += static_cast<double>(value);
    }

    return total / values.size();
}

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
);
