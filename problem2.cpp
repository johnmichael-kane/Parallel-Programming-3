#include <climits>
#include <iostream>
#include <mutex>
#include <set>
#include <thread>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>

static const int sensor_count = 8;
static const int minutes = 60;
static const int hours = 24;

std::mutex mutex;

int generateRandomTemperature(int min, int max) {
    static thread_local std::mt19937       generator(std::random_device{}());
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

bool areAllSensorsReady(int thisSensor, const std::vector<bool>& sensorStates) {
    for (size_t i = 0; i < sensorStates.size(); ++i) {
        if (!sensorStates[i] && static_cast<int>(i) != thisSensor) {
            return false;
        }
    }
    return true;
}

void analyzeTemperatureDifferences(const std::vector<int>& readings) {
    const int checkInterval = 10;
    int maxDiff = INT_MIN;
    int intervalStart = 0;

    for (int i = 0; i <= static_cast<int>(readings.size()) - checkInterval; ++i) {
        int localMax = *std::max_element(readings.begin() + i, readings.begin() + i + checkInterval);
        int localMin = *std::min_element(readings.begin() + i, readings.begin() + i + checkInterval);
        int diff = localMax - localMin;

        if (diff > maxDiff) {
            maxDiff = diff;
            intervalStart = i;
        }
    }

    std::cout << "Max difference: " << maxDiff << "F from " 
              << intervalStart << "-" << (intervalStart + checkInterval) << " min."<< std::endl;
}

void displayExtremes(const std::vector<int>& sortedReadings) {
    std::cout << "Top 5 Highest Temperatures: ";
    for (int i = 0; i < 5 && i < sortedReadings.size(); ++i) {
        std::cout << sortedReadings[sortedReadings.size() - 1 - i] << "F ";
    }

    std::cout << "\nTop 5 Lowest temperatures: ";
    for (int i = 0; i < 5 && i < sortedReadings.size(); ++i) {
        std::cout << sortedReadings[i] << "F ";
    }
    std::cout << std::endl;
}

void compileHourlyReport(int hour, const std::vector<int>& readings) {
    std::cout << "\nHour " << hour + 1 << " Report:\n";
    analyzeTemperatureDifferences(readings);
    auto sortedReadings = readings; // copies the readings to sort
    std::sort(sortedReadings.begin(), sortedReadings.end());
    displayExtremes(sortedReadings);
}

void recordSensorData(int sensorId, std::vector<int>& readings, std::vector<bool>& readyFlags) {
    for (int hour = 0; hour < hours; ++hour) {
        for (int minute = 0; minute < minutes; ++minute) {
            int readingIdx = sensorId * minutes + minute;
            readings[readingIdx] = generateRandomTemperature(-100, 70);
            readyFlags[sensorId] = true;

            std::this_thread::sleep_for(std::chrono::milliseconds(1)); // simulates the reading time

            while (!areAllSensorsReady(sensorId, readyFlags)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }

        if (sensorId == 0) { // First sensor compiles the report
            std::lock_guard<std::mutex> lock(mutex);
            compileHourlyReport(hour, readings);
        }
    }
}

int main() {
    std::vector<int> readings(sensor_count * minutes, 0);
    std::vector<bool> readyFlags(sensor_count, false);
    std::thread sensors[sensor_count];

    for (int i = 0; i < sensor_count; ++i) {
        sensors[i] = std::thread(recordSensorData, i, std::ref(readings), std::ref(readyFlags));
    }

    auto startTime = std::chrono::high_resolution_clock::now();

    for (auto& sensor : sensors) {
        sensor.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed = endTime - startTime;
    std::cout << "Completed in " << elapsed.count() << "ms\n";
}
