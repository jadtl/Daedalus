#include "daedalus/engine.h"

#include <string>
#include <vector>

int main()
{
    std::vector<std::string> args;
    args.push_back("-validate");

    Engine *engine = new Engine(args);
    engine->initialize();

    while (!glfwWindowShouldClose(engine->window))
    {
        engine->update();
        engine->render();
        glfwPollEvents();
    }

    return 0;
}