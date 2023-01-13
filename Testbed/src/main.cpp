#include <daedalus.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <memory>

using namespace ddls;

const char *appName = "Daedalus";
const char *engineName = "Daedalus";

const u32 width = 1200;
const u32 height = 1024;

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow *window = glfwCreateWindow(width, height, appName, nullptr, nullptr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsClassic();

    //ImGuiIO &io = ImGui::GetIO();
		//(void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    //ImGuiStyle& style = ImGui::GetStyle();
    //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    //{
        //style.WindowRounding = 0.0f;
        //style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    //}

    ImGui_ImplGlfw_InitForVulkan(window, true);

    std::unique_ptr<ddls::Renderer> renderer = std::make_unique<ddls::Renderer>(window, appName, engineName);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = renderer->instance();
    initInfo.PhysicalDevice = renderer->physicalDevice();
    initInfo.Device = renderer->device();
    initInfo.Queue = renderer->queue();
    initInfo.DescriptorPool = renderer->descriptorPoolImGui();
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = renderer->imageCount();
    ImGui_ImplVulkan_Init(&initInfo, renderer->renderPassImGui());

    VkCommandPool commandPool = renderer->commandPoolImGui();
    VkCommandBuffer commandBuffer = renderer->commandBufferImGui();

    vkResetCommandPool(renderer->device(), commandPool, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkEndCommandBuffer(commandBuffer);

    Assert(vkQueueSubmit(renderer->queue(), 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS,
        "Failed to submit to queue!");

    vkDeviceWaitIdle(renderer->device());

    ImGui_ImplVulkan_DestroyFontUploadObjects();

    renderer->wireframe = false;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();

        ImGui::Begin("Config");
        ImGui::Checkbox("Wireframe", &renderer->wireframe);
        ImGui::SliderFloat("R", &renderer->red, 0.0, 1.0f);
        ImGui::SliderFloat("G", &renderer->green, 0.0, 1.0f);
        ImGui::SliderFloat("B", &renderer->blue, 0.0, 1.0f);
        ImGui::SliderFloat("Rotate", &renderer->rotate, 0.0, glm::radians(90.0));
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        ImGui::Render();
        ImDrawData *drawData = ImGui::GetDrawData();
        // Update and Render additional Platform Windows
        //if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        //{
            //ImGui::UpdatePlatformWindows();
            //ImGui::RenderPlatformWindowsDefault();
        //}
        renderer->render(drawData);
    }

    vkDeviceWaitIdle(renderer->device());

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    renderer.reset();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}