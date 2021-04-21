#pragma once

#include <boost/filesystem.hpp>

#if defined(_WIN32)
#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd

#else
#include <unistd.h>
#define GetCurrentDir getcwd

#if defined(__APPLE__)
#include <mach-o/dyld.h>

#elif defined(__linux__)
#include <sstream>
#include <unistd.h>
#endif
#endif

#include <string>

class Explorer {
public:
    Explorer(const std::string& pathFromExecutable);

    const std::string& shader(const std::string& fileName);
    const std::string& asset(const std::string& fileName);

private:
    boost::filesystem::path executable();
    boost::filesystem::path programRoot;

    const std::string& currentWorkingDirectory();
};