#pragma once

#include <boost/filesystem.hpp>

#if defined(_WIN32)
  #include <windows.h>
#elif defined(__linux__)
  #include <sstream>
  #include <unistd.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
#endif

#ifdef WINDOWS
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#include <string>

class Shell {
public:
    Shell(std::string pathFromExecutable);
    
    boost::filesystem::path shaders();
    boost::filesystem::path assets();
    
private:
    boost::filesystem::path executable();
    boost::filesystem::path programRoot;
    std::string currentWorkingDirectory();
};
