#include "Explorer.h"

#include <windows.h>
#include <direct.h>
#define GetCurrentDir _getcwd

class ExplorerWindows : public Explorer {
private:
    boost::filesystem::path executable();
    const std::string& currentWorkingDirectory();
};
