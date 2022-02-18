#include <daedalus.h>

#include <string>
#include <vector>

int main()
{
    std::vector<std::string> args;
    args.push_back("-validate");

    ddls::Engine* engine = new ddls::Engine(args);
    engine->initialize();

    while (!glfwWindowShouldClose(engine->window))
    {
        engine->update();
        engine->render();
        glfwPollEvents();
    }

    delete engine;

    return 0;
}