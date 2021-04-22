#include "Platform.h"

Platform::Platform(std::string pathFromExecutable) {
    programRoot =  boost::filesystem::relative(executable().remove_filename().append(pathFromExecutable), currentWorkingDirectory());
}

std::string Platform::shader(std::string fileName) { return programRoot.string() + "/Shaders/" + fileName; }

std::string Platform::asset(std::string fileName) { return programRoot.string() + "/Assets/" + fileName; }

boost::filesystem::path Platform::executable() {
  unsigned int bufferSize = 512;
  std::vector<char> buffer(bufferSize + 1);

#if defined(_WIN32)
  ::GetModuleFileName(NULL, (LPWSTR)&buffer[0], bufferSize);

#elif defined(__linux__)
  // Get the process ID.
  int pid = getpid();

  // Construct a path to the symbolic link pointing to the process executable.
  // This is at /proc/<pid>/exe on Linux systems (we hope).
  std::ostringstream oss;
  oss << "/proc/" << pid << "/exe";
  std::string link = oss.str();

  // Read the contents of the link.
  int count = readlink(link.c_str(), &buffer[0], bufferSize);
  if(count == -1) throw std::runtime_error("Could not read symbolic link");
  buffer[count] = '\0';

#elif defined(__APPLE__)
  if(_NSGetExecutablePath(&buffer[0], &bufferSize))
  {
    buffer.resize(bufferSize);
    _NSGetExecutablePath(&buffer[0], &bufferSize);
  }

#else
  #error Cannot yet find the executable on this platform
#endif

  std::string s = &buffer[0];
  return s;
}

std::string Platform::currentWorkingDirectory() {
   char buffer[FILENAME_MAX];
   GetCurrentDir(buffer, FILENAME_MAX);
   std::string currentDirectory(buffer);
   return currentDirectory;
}
