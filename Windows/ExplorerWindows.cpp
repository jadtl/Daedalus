#include "ExplorerWindows.h"

boost::filesystem::path Explorer::executable() {
    unsigned int bufferSize = 512;
    std::vector<char> buffer(bufferSize + 1);
    
    ::GetModuleFileName(NULL, (LPWSTR)&buffer[0], bufferSize);
    
    std::string executablePath = &buffer[0];
    return executablePath;
}

const std::string& Explorer::currentWorkingDirectory() {
    char buffer[FILENAME_MAX];
    auto tmp = GetCurrentDir(buffer, FILENAME_MAX);
    std::string currentDirectory(buffer);
    return currentDirectory;
}
