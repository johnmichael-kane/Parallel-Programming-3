#include <algorithm>
#include <climits>
#include <iostream>
#include <mutex>
#include <random>
#include <set>
#include <thread>
#include <vector>
#include <condition_variable> 

class TemperatureMonitoringSystem {
private:
    static const int sensor_count = 8;
    static const int minutes = 60;
    static const int hours = 24;
    std::vector<int> sensorReadings;
    std::vector<bool> sensorsReady;
    std::mutex mutex;
    std::condition_variable cv;
    int reportHour = -1; // Tracks the current hour for which the report is being generated

public:
    TemperatureMonitoringSystem() : sensorReadings(sensor_count * minutes), sensorsReady(sensor_count, false) {}

    void run() {
        std::vector<std::thread> threads;
        for (int i = 0; i < sensor_count; ++i) {
            threads.emplace_back(&TemperatureMonitoringSystem::recordTemperatureReadings, this, i);
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

private:
    static int generateRandomNumber(int min, int max) {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<std::mt19937::result_type> dist(min, max);
        return dist(rng);
    }

    void recordTemperatureReadings(int threadId) {
        for (int hour = 0; hour < hours; ++hour) {
            for (int minute = 0; minute < minutes; ++minute) {
                int index = hour * minutes + minute;
                sensorReadings[index] = generateRandomNumber(-100, 70);

                {
                    std::unique_lock<std::mutex> lk(mutex);
                    sensorsReady[threadId] = true;
                    cv.notify_all();
                    // Wait for all sensors to be ready
                    cv.wait(lk, [&]{ return allSensorsReady(); });
                }

                if (threadId == 0) {
                    compileHourlyReport(hour);
                }

                // Reset readiness for the next minute
                sensorsReady[threadId] = false;
            }
        }
    }

    bool allSensorsReady() const {
        return std::all_of(sensorsReady.begin(), sensorsReady.end(), [](bool ready) { return ready; });
    }

    void compileHourlyReport(int hour) {
        std::lock_guard<std::mutex> lock(mutex);
        if (hour <= reportHour) return; // Ensure we don't generate the report multiple times for the same hour

        reportHour = hour;
        printLargestDifference();
        printHighestTemperatures();
        printLowestTemperatures();
    }

    void printLargestDifference() {
        int step = 10;
        int startInterval = 0;
        int maxDifference = INT_MIN;
        int endInterval = step;

        for (int i = 0; i <= sensorReadings.size() - step; ++i) {
            int localMax = *std::max_element(sensorReadings.begin() + i, sensorReadings.begin() + i + step);
            int localMin = *std::min_element(sensorReadings.begin() + i, sensorReadings.begin() + i + step);
            int localDiff = localMax - localMin;

            if (localDiff > maxDifference) {
                maxDifference = localDiff;
                startInterval = i;
                endInterval = i + step;
            }
        }

        std::cout << "Largest temperature difference: " << maxDifference << "F"
                  << " from minute " << startInterval << " to minute " << endInterval << "." << std::endl;
    }

    void printHighestTemperatures() {
        std::set<int, std::greater<int>> temperatures(sensorReadings.begin(), sensorReadings.end());
        std::cout << "Highest temperatures: ";
        int count = 0;
        for (int temp : temperatures) {
            std::cout << temp << "F ";
            if (++count == 5) break;
        }
        std::cout << std::endl;
    }

    void printLowestTemperatures() {
        std::set<int> temperatures(sensorReadings.begin(), sensorReadings.end());
        std::cout << "Lowest temperatures: ";
        int count = 0;
        for (int temp : temperatures) {
            std::cout << temp << "F ";
            if (++count == 5) break;
        }
        std::cout << std::endl;
    }
};

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    TemperatureMonitoringSystem system;
    system.run();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "Finished in " << duration.count() << "ms" << std::endl;
}
