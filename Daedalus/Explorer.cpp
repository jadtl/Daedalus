#include "Explorer.h"

std::string Explorer::shader(std::string fileName) const { return (programRoot.string() + "/Shaders/" + std::string(fileName)); }

std::string Explorer::asset(std::string fileName) const { return (programRoot.string() + "/Assets/" + std::string(fileName)); }
