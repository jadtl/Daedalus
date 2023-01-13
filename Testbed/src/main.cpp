#include <daedalus.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <memory>

using namespace ddls;

const char *appName = "Daedalus";
const char *engineName = "Daedalus";

const u32 width = 800;
const u32 height = 600;

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(width, height, appName, nullptr, nullptr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsClassic();

    ImGui_ImplGlfw_InitForVulkan(window, true);

    std::unique_ptr<ddls::Renderer> renderer = std::make_unique<ddls::Renderer>(window, appName, engineName);

    ImGui_ImplVulkan_InitInfo init_info{};
    //init_info.Instance = 

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        renderer->render();
    }

    vkDeviceWaitIdle(renderer->device());

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}