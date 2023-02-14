#pragma once

#include "core/defines.h"
#include "core/types.h"

#include <map>
#include <filesystem>

namespace ddls {

/**
 * @brief A resource management class
 *
 */
class DDLS_API Resources
{
public:
    ~Resources();

    /**
     * @brief Returns the single instance of the class
     *
     */
    static Resources& Manager()
    {
        static Resources manager;

        return manager;
    }
    Resources(Resources const&)       = delete;
    void operator=(Resources const&)  = delete;

    /**
     * @brief Gets the requested resource, allocating its contents if needed
     *
     */
    const char* get(const char* filePath);

    /**
     * @brief Frees the given resource
     *
     */
    void free(const char* filePath);

private:
    Resources() = default;
    std::map<const char*, char*> allocations;
    static std::filesystem::path cwd();
};

} // namespace ddls
