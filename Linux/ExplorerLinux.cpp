#include "ExplorerLinux.h"

boost::filesystem::path Explorer::executable() {
    unsigned int bufferSize = 512;
    std::vector<char> buffer(bufferSize + 1);
    
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
    
    std::string executablePath = &buffer[0];
    return executablePath;
}

const std::string& Explorer::currentWorkingDirectory() {
    char buffer[FILENAME_MAX];
    auto tmp = GetCurrentDir(buffer, FILENAME_MAX);
    std::string currentDirectory(buffer);
    return currentDirectory;
}
