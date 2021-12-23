#include <math/linalg.h>

#include <math.h>
#include <stdlib.h>
#include <chrono>

static b8 rand_seeded = false;

namespace ddls {
    using namespace std::chrono;
    /**
     * Note that these are here in order to prevent having to import the
     * entire <linalg.h> everywhere.
     */
    f32 sin(f32 x)
    {
        return sinf(x);
    }

    f32 cos(f32 x)
    {
        return cosf(x);
    }

    f32 tan(f32 x)
    {
        return tanf(x);
    }

    f32 acos(f32 x)
    {
        return acosf(x);
    }

    f32 sqrt(f32 x)
    {
        return sqrtf(x);
    }

    f32 abs(f32 x)
    {
        return fabsf(x);
    }

    i32 random()
    {
        if (!rand_seeded)
        {
            srand((u32)duration<f64>(high_resolution_clock::now().time_since_epoch()).count());
            rand_seeded = true;
        }
        return rand();
    }

    i32 random_in_range(i32 min, i32 max)
    {
        if (!rand_seeded)
        {
            srand((u32)duration<f64>(high_resolution_clock::now().time_since_epoch()).count());
            rand_seeded = true;
        }
        return (rand() % (max - min + 1)) + min;
    }

    f32 frandom()
    {
        return (float)random() / (f32)RAND_MAX;
    }

    f32 frandom_in_range(f32 min, f32 max)
    {
        return min + ((float)random() / ((f32)RAND_MAX / (max - min)));
    }
}