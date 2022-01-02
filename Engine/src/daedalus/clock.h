#pragma once

#include <defines.h>

#include <chrono>

namespace ddls {
    using namespace std::chrono;

    /**
     * @brief A clock class used by Daedalus
     */
    class DDLS_API Clock {
    public:
        Clock();
        // Updates the provided clock. Should be called just before checking elapsed time.
        // Has no effect on non-started clocks.
        void update();
        // Starts the provided clock. Resets elapsed time.
        void start();
        // Stops the provided clock. Does not reset elapsed time.
        void stop();

    private:
        time_point<high_resolution_clock> start_time;
        f64 elapsed_time;
    };
}