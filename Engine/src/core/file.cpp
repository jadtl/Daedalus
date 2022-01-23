#include <core/file.h>

namespace ddls {
std::string File::Shader(std::string file_name) { return File::Cwd().parent_path().parent_path().append("Shaders").append(file_name).string(); }

std::string File::Asset(std::string file_name) { return File::Cwd().parent_path().parent_path().append("Assets").append(file_name).string(); }

std::filesystem::path File::Cwd()
{
    char buffer[FILENAME_MAX];
    GetCurrentDir(buffer, FILENAME_MAX);
    std::string currentDirectory(buffer);
    return currentDirectory;
}
}