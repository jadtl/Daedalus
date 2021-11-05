#include "Explorer.h"

#include <mach-o/dyld.h>
#include <unistd.h>
#define GetCurrentDir getcwd

class ExplorerDarwin : public Explorer {
public:
    ExplorerDarwin(std::string pathFromExecutable);
    ~ExplorerDarwin();
private:
    boost::filesystem::path executable();
    std::string currentWorkingDirectory();
};
