#pragma once

#include "defines.h"

#include <filesystem>
#include <string>

#if defined(DDLS_PLATFORM_WINDOWS)
#include <direct.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

namespace ddls {
class File
{
public:
  static std::string Shader(std::string file_name);
  static std::string Asset(std::string file_name);
  static std::filesystem::path Cwd();
};
}