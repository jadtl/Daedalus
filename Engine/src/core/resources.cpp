#include "core/resources.h"

#include "core/log.h"

#include "core/assert.h"

namespace ddls {

Resources::~Resources()
{
	for (auto & allocation : allocations)
	{
		::free(&(allocation.second));
	}
}

const char* Resources::get(const char* filePath)
{
	if (allocations.count(filePath)) return allocations[filePath];
	Log::Debug(cwd().string());
	std::ifstream file(cwd().append(filePath), std::ios::ate | std::ios::binary);

	Assert(file.is_open(), 
		fmt::format("Cannot open file \"{}\"!", filePath));

	u32 fileSize = (u32)file.tellg();
	char *buffer = (char*)malloc(fileSize);

	file.seekg(0);
	file.read(buffer, fileSize);

	file.close();
	return buffer;
}

void Resources::free(const char* filePath)
{
	if (allocations.count(filePath))
	{
		::free(allocations[filePath]);
		allocations.erase(filePath);
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
