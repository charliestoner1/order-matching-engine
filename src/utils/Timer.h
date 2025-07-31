#pragma once

#include <chrono>
#include <iostream>

namespace order_matching {
    namespace utils {

        class Timer {
        private:
            std::chrono::high_resolution_clock::time_point start_time;

        public:

            Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

            void start() {
                start_time = std::chrono::high_resolution_clock::now();
            }

            double elapsed_microseconds() const {
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                return duration.count();
            }

            double elapsed_milliseconds() const {
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                return duration.count();
            }
        };

        // scoped timer for easy benchmarking
        class ScopedTimer {
        private:
            std::string name;
            Timer timer;

        public:
            ScopedTimer(const std::string& timer_name) : name(timer_name) {
                std::cout << "[" << name << "] Starting..." << std::endl;
            }

            ~ScopedTimer() {
                double elapsed = timer.elapsed_microseconds();
                std::cout << "[" << name << "] Completed in " << elapsed << " microseconds" << std::endl;
            }
        };

    } // namespace utils
} // namespace order_matching