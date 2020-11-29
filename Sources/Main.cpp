#include "../Headers/DaedalusEngine.hpp"

#include <stdexcept>
#include <cstdlib>

#ifdef _WIN32
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int main()
{
    ddls::DaedalusEngine mainApplication(800, 600, "Daedalus");
    try {
        mainApplication.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
#ifdef _WIN32
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    return main();
}
#endif