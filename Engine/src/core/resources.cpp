#include "core/resources.h"

#include "core/log.h"
#include "core/assert.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace ddls {

Resources::~Resources()
{
	for (auto & allocation : _files)
	{
		::free(&(allocation.second));
	}
}

std::filesystem::path Resources::getPath(const char* filePath)
{
    return cwd().append(filePath);
}

const char* Resources::getFile(const char* filePath)
{
	if (_files.count(filePath)) return _files[filePath];
	std::ifstream file(cwd().append(filePath), std::ios::ate | std::ios::binary);

	Assert(file.is_open(), 
		fmt::format("Cannot open file \"{}\"!", filePath));

	u32 fileSize = (u32)file.tellg();
	char *buffer = (char*)malloc(fileSize);

	file.seekg(0);
	file.read(buffer, fileSize);

	file.close();

	_files[filePath] = buffer;
	return buffer;
}

const Texture Resources::getTexture(const char* texturePath)
{
	if (_textures.count(texturePath)) return _textures[texturePath];

	Texture tex{};
	int width, height, channels;
	stbi_set_flip_vertically_on_load(true);
	tex.data = stbi_load(texturePath, &width, &height, &channels, 0);
	Assert(tex.data != nullptr,
		fmt::format("Failed to get texture \"{}\"!", texturePath));
	tex.width = (u16)width;
	tex.height = (u16)height;
	tex.channels = (u16)channels;

	_textures[texturePath] = tex;
	return tex;
}

void Resources::free(const char* filePath)
{
	if (_files.count(filePath))
	{
		::free(_files[filePath]);
		_files.erase(filePath);
	}

	if (_textures.count(filePath))
	{
		::free(_textures[filePath].data);
		_textures.erase(filePath);
	}
}

#ifdef DDLS_PLATFORM_WINDOWS

#include <Windows.h>

std::filesystem::path Resources::cwd()
{
	u32 size = FILENAME_MAX;
	char buffer[size];
	GetModuleFileNameA(nullptr, buffer, size);
	std::string currentDirectory(buffer);
	return ((std::filesystem::path)currentDirectory).parent_path();
}

#elif DDLS_PLATFORM_MACOS

#include <mach-o/dyld.h>

std::filesystem::path Resources::cwd()
{
	u32 size = FILENAME_MAX;
	char buffer[size];
	_NSGetExecutablePath(buffer, &size);
	
	std::string currentDirectory(buffer);
	// Supposes that the executable is bundled
	return ((std::filesystem::path)currentDirectory).parent_path().parent_path().append("Resources");
}

#elif DDLS_PLATFORM_LINUX

#include <unistd.h>

std::filesystem::path Resources::cwd()
{
	u32 size = FILENAME_MAX;
	char buffer[size];
	readlink("/proc/self/exe", buffer, size);
	std::string currentDirectory(buffer);
	return ((std::filesystem::path)currentDirectory).parent_path();
}

#endif

} // namespace ddls
