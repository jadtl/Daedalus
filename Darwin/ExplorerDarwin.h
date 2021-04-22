#include "Explorer.h"

#include <mach-o/dyld.h>
#include <unistd.h>
#define GetCurrentDir getcwd

class ExplorerDarwin : public Explorer {
private:
    boost::filesystem::path executable();
    const std::string& currentWorkingDirectory();
};
