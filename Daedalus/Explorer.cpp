#include "Explorer.h"

Explorer::Explorer(const std::string& pathFromExecutable) {
    programRoot = boost::filesystem::relative(executable().remove_filename().append(pathFromExecutable), currentWorkingDirectory());
}

const std::string& Explorer::shader(const std::string& fileName) { return (programRoot.string() + "/Shaders/" + std::string(fileName)); }

const std::string& Explorer::asset(const std::string& fileName) { return (programRoot.string() + "/Assets/" + std::string(fileName)); }

boost::filesystem::path Explorer::executable() {
    unsigned int bufferSize = 512;
    std::vector<char> buffer(bufferSize + 1);

#if defined(_WIN32)
    ::GetModuleFileName(NULL, (LPWSTR)&buffer[0], bufferSize);

#elif defined(__APPLE__)
    if (_NSGetExecutablePath(&buffer[0], &bufferSize)) {
        buffer.resize(bufferSize);
        _NSGetExecutablePath(&buffer[0], &bufferSize);
    }

#elif defined(__linux__)
    // Get the process ID.
    int pid = getpid();

    // Construct a path to the symbolic link pointing to the process executable.
    // This is at /proc/<pid>/exe on Linux systems (we hope).
    std::ostringstream oss;
    oss << "/proc/" << pid << "/exe";
    std::string link = oss.str();

    // Read the contents of the link.
    int count = readlink(link.c_str(), &buffer[0], bufferSize);
    if (count == -1) throw std::runtime_error("Could not read symbolic link");
    buffer[count] = '\0';

#else
#error Cannot yet find the executable on this platform
#endif

    std::string executablePath = &buffer[0];
    return executablePath;
}

const std::string& Explorer::currentWorkingDirectory() {
    char buffer[FILENAME_MAX];
    auto tmp = GetCurrentDir(buffer, FILENAME_MAX);
    std::string currentDirectory(buffer);
    return currentDirectory;
}
