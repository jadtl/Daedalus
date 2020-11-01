#include "DaedalusEngine.hpp"

#include <stdexcept>
#include <cstdlib>

int main()
{
    ddls::DaedalusEngine mainApplication(800, 600, "Daedalus' Original Work");
    try {
        mainApplication.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
