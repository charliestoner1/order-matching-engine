#pragma once

#include <chrono>
#include <string>
#include <iostream>
#include <functional>

namespace order_matching::utils {

// High-precision timer for performance measurement
class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::nanoseconds;

    Timer() : start_time_(Clock::now()) {}

    // Start or restart the timer
    void start() noexcept {
        start_time_ = Clock::now();
    }

    // Get elapsed time in nanoseconds
    [[nodiscard]] Duration elapsed() const noexcept {
        return std::chrono::duration_cast<Duration>(Clock::now() - start_time_);
    }

    // Get elapsed time in microseconds
    [[nodiscard]] double elapsed_microseconds() const noexcept {
        return std::chrono::duration<double, std::micro>(elapsed()).count();
    }

    // Get elapsed time in milliseconds
    [[nodiscard]] double elapsed_milliseconds() const noexcept {
        return std::chrono::duration<double, std::milli>(elapsed()).count();
    }

    // Reset and return elapsed time
    [[nodiscard]] Duration reset() noexcept {
        auto elapsed_time = elapsed();
        start();
        return elapsed_time;
    }

private:
    TimePoint start_time_;
};

// RAII timer that prints elapsed time on destruction
class ScopedTimer {
public:
    explicit ScopedTimer(const std::string& name)
        : name_(name), timer_() {
        std::cout << "[" << name_ << "] Starting...\n";
    }

    ~ScopedTimer() {
        auto elapsed = timer_.elapsed_microseconds();
        std::cout << "[" << name_ << "] Completed in "
                  << elapsed << " microseconds\n";
    }

    // Delete copy/move to ensure single timing
    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;
    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

private:
    std::string name_;
    Timer timer_;
};

// Forward declarations for Timer.cpp functions
std::string format_duration(Timer::Duration duration);
void benchmark_operation(const std::string& name,
                        std::function<void()> operation,
                        size_t iterations = 1000);

} // namespace order_matching::utils