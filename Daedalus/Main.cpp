#include "Engine.h"

#include <string>
#include <vector>

namespace {
	Engine* createEngine(const std::vector<std::string>& args, void* windowHandle) { return new Engine(args, windowHandle); }

	Engine* createEngine(int argc, char** argv, void* windowHandle) {
		std::vector<std::string> args(argv, argv + argc);
		return createEngine(args, nullptr);
	}
}

#if defined(_WIN32)
int main(int argc, char** argv) {
	Engine *engine = createEngine(argc, argv, nullptr);
	delete engine;

	return 0;
}
#endif