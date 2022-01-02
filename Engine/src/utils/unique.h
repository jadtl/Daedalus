#pragma once

#include <defines.h>

namespace ddls {
    /**
     * @brief A uniqueness class that removes the copy constructor and operator from derived classes, while leaving move.
     */
    class DDLS_API Unique {
    protected:
        Unique() = default;
        virtual ~Unique() = default;

    public:
        Unique(const Unique&) = delete;
        Unique(Unique&&) noexcept = default;
        Unique& operator=(const Unique&) = delete;
        Unique& operator=(Unique&&) noexcept = default;
    };
}