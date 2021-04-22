#include "Explorer.h"

#include <sstream>
#include <unistd.h>
#define GetCurrentDir getcwd

class ExplorerLinux : public Explorer {
private:
    boost::filesystem::path executable();
    const std::string& currentWorkingDirectory();
};
