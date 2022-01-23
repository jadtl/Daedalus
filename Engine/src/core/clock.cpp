#include <core/clock.h>

#include <limits>

namespace ddls {
    using namespace std::chrono;

    constexpr f64 f64_min = std::numeric_limits<f64>::min();

    Clock::Clock() {
        this->elapsed_time = f64_min;
    }

    void Clock::update() {
        if (this->elapsed_time != f64_min)
            this->elapsed_time = duration<f64>(this->start_time - high_resolution_clock::now()).count();
    }

    void Clock::start() {
        this->start_time = high_resolution_clock::now();
        this->elapsed_time = 0;
    }

    void Clock::stop() {
        this->elapsed_time = f64_min;
    }
}