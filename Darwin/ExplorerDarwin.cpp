#include "ExplorerDarwin.h"

ExplorerDarwin::ExplorerDarwin(std::string pathFromExecutable) : Explorer(*this) {
    programRoot = boost::filesystem::relative(executable().remove_filename().append(pathFromExecutable), currentWorkingDirectory());
}

ExplorerDarwin::~ExplorerDarwin() {}

boost::filesystem::path ExplorerDarwin::executable() {
    unsigned int bufferSize = 512;
    std::vector<char> buffer(bufferSize + 1);
    
    if (_NSGetExecutablePath(&buffer[0], &bufferSize)) {
        buffer.resize(bufferSize);
        _NSGetExecutablePath(&buffer[0], &bufferSize);
    }
    
    std::string executablePath = &buffer[0];
    return executablePath;
}

std::string ExplorerDarwin::currentWorkingDirectory() {
    char buffer[FILENAME_MAX];
    auto tmp = GetCurrentDir(buffer, FILENAME_MAX);
    std::string currentDirectory(buffer);
    return currentDirectory;
}
