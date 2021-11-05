#pragma once

#include <boost/filesystem.hpp>
#include <vulkan/vulkan.h>

#if defined(_WIN32)
  #include <windows.h>
#elif defined(__linux__)
  #include <sstream>
  #include <unistd.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
#endif

#if defined(_WIN32)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <string>

class Platform {
public:
    Platform(std::string pathFromExecutable);
    
    std::string shader(std::string fileName);
    std::string asset(std::string fileName);
    
private:
    boost::filesystem::path executable();
    boost::filesystem::path programRoot;
    
    std::string currentWorkingDirectory();
};
