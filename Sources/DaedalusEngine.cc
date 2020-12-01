#include <DaedalusEngine.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace ddls
{
DaedalusEngine::DaedalusEngine()
{
    applicationName = "Undefined Application";
    
    windowWidth = 800;
    windowHeight = 600;
    
    currentFrame = 0;
    
    framebufferResized = false;
}

DaedalusEngine::DaedalusEngine(uint32_t windowWidth_p,
                               uint32_t windowHeight_p,
                               std::string applicationName_p)
{
    applicationName = applicationName_p;
    
    windowWidth = windowWidth_p;
    windowHeight= windowHeight_p;

    currentFrame = 0;

    framebufferResized = false;
}

void DaedalusEngine::run()
{
    initSystem();
    initVulkan();
    mainLoop();
    cleanup();
}

void DaedalusEngine::initSystem()
{
    glfwInit();
    
    // Tell not to Create OpenGL Context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    // 4th Specifies which Monitor to Use
    // 5th Only Relevant to OpenGL
    window = glfwCreateWindow(windowWidth, windowWidth, applicationName.c_str(), 0, nullptr);
    
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    GLFWimage images[1];
    images[0].pixels = stbi_load("Resources/Daedalus.png", &images[0].width, &images[0].height, 0, 4);
    glfwSetWindowIcon(window, 1, images);
    stbi_image_free(images[0].pixels);

    soundEngine.init();
    backgroundMusic.load("Sounds/GetYourWish.mp3");
    int backgroundMusicHandle = soundEngine.play(backgroundMusic);
    soundEngine.setVolume(backgroundMusicHandle, 0.25f);
}

void DaedalusEngine::initVulkan()
{
    createInstance
    (instance, validationLayers, applicationName, engineName);
    
    setupDebugMessenger
    (instance, &debugMessenger);
    
    createSurface
    (instance, window, &surface);
    
    selectPhysicalDevice
    (instance, physicalDevice, surface);
    
    createLogicalDevice
    (device, physicalDevice, graphicsQueue, presentQueue, surface, validationLayers);
    
    createSwapChain
    (device, physicalDevice, surface, swapChain, swapChainImages, swapChainImageFormat, swapChainExtent, window, windowWidth, windowHeight);
    
    createImageViews
    (device, swapChainImageViews, swapChainImages, swapChainImageFormat);
    
    createRenderPass
    (device, physicalDevice, renderPass, swapChainImageFormat);
    
    createDescriptorSetLayout
    (device, descriptorSetLayout);
    
    createGraphicsPipeline
    (device, pipelineLayout, graphicsPipeline, renderPass, swapChainExtent, descriptorSetLayout);
    
    createCommandPool
    (device, physicalDevice, surface, commandPool);
    
    createDepthResources
    (device, physicalDevice, depthImage, depthImageMemory, depthImageView, swapChainExtent);
    
    createFrameBuffers
    (device, swapChainFrameBuffers, renderPass, swapChainImageViews, swapChainExtent, depthImageView);
    
    createTextureImage
    (physicalDevice, device, textureImage, textureImageMemory, commandPool, graphicsQueue);
    
    createTextureImageView
    (device, textureImage, textureImageView);
    
    createTextureSampler
    (device, textureSampler);
    
    loadModel
    (vertices, indices);
    
    createVertexBuffers
    (physicalDevice, device, vertexBuffer, vertexBufferMemory, commandPool, graphicsQueue, vertices);
    
    createIndexBuffers
    (physicalDevice, device, indexBuffer, indexBufferMemory, commandPool, graphicsQueue, indices);
    
    createUniformBuffers
    (physicalDevice, device, uniformBuffers, uniformBuffersMemory, swapChainImages);
    
    createDescriptorPool
    (device, descriptorPool, swapChainImages);
    
    createDescriptorSets
    (device, uniformBuffers, descriptorSetLayout, descriptorPool, descriptorSets, swapChainImages, textureImageView, textureSampler);

    createCommandBuffers
    (device, commandBuffers, commandPool, graphicsPipeline, swapChainFrameBuffers, renderPass, swapChainExtent, vertexBuffer, indexBuffer, pipelineLayout, descriptorSets, indices);
    
    createSyncObjects
    (device, imageAvailableSemaphores, renderFinishedSemaphores, inFlightFences, imagesInFlight, swapChainImages);
}

void DaedalusEngine::mainLoop()
{
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame();
    }
    
    vkDeviceWaitIdle(device);
}

void DaedalusEngine::cleanupSwapChain()
{
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    for (auto framebuffer : swapChainFrameBuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
    
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
    
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void DaedalusEngine::cleanup()
{
    cleanupSwapChain();
    
    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);
    
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);
    
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    
    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);
    
    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);
    
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }
    
    vkDestroyCommandPool(device, commandPool, nullptr);
    
    vkDestroyDevice(device, nullptr);
    
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    soundEngine.deinit();

    glfwDestroyWindow(window);

    glfwTerminate();
}

void DaedalusEngine::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    
    vkDeviceWaitIdle(device);

    createSwapChain
    (device, physicalDevice, surface, swapChain, swapChainImages, swapChainImageFormat, swapChainExtent, window, windowWidth, windowHeight);
    
    createImageViews
    (device, swapChainImageViews, swapChainImages, swapChainImageFormat);
    
    createRenderPass
    (device, physicalDevice, renderPass, swapChainImageFormat);
    
    createGraphicsPipeline
    (device, pipelineLayout, graphicsPipeline, renderPass, swapChainExtent, descriptorSetLayout);
    
    createDepthResources
    (device, physicalDevice, depthImage, depthImageMemory, depthImageView, swapChainExtent);
    
    createFrameBuffers
    (device, swapChainFrameBuffers, renderPass, swapChainImageViews, swapChainExtent, depthImageView);
    
    createUniformBuffers
    (physicalDevice, device, uniformBuffers, uniformBuffersMemory, swapChainImages);
    
    createDescriptorPool
    (device, descriptorPool, swapChainImages);
    
    createDescriptorSets
    (device, uniformBuffers, descriptorSetLayout, descriptorPool, descriptorSets, swapChainImages, textureImageView, textureSampler);
    
    createCommandBuffers
    (device, commandBuffers, commandPool, graphicsPipeline, swapChainFrameBuffers, renderPass, swapChainExtent, vertexBuffer, indexBuffer, pipelineLayout, descriptorSets, indices);
}

void DaedalusEngine::drawFrame()
{
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swap chain image!");
    }
    
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
    
    updateUniformBuffer(device, swapChainExtent, uniformBuffersMemory, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
}
