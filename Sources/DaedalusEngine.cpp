#include "DaedalusEngine.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace ddls
{
DaedalusEngine::DaedalusEngine()
{
    applicationName = "Undefined Application";
    
    windowWidth = 800;
    windowHeight = 600;
    
    physicalDevice = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
    
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
    
    physicalDevice = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
    
    currentFrame = 0;
    
    framebufferResized = false;
}

void DaedalusEngine::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void DaedalusEngine::initWindow()
{
    glfwInit();
    
    // Tell not to Create OpenGL Context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    // 4th Specifies which Monitor to Use
    // 5th Only Relevant to OpenGL
    window = glfwCreateWindow(windowWidth, windowWidth, applicationName.c_str(), 0, nullptr);
    
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void DaedalusEngine::initVulkan()
{
    createInstance
    (instance, validationLayers, applicationName, engineName);
    
    setupDebugMessenger
    (instance, &debugMessenger);
    
    createSurface
    (instance, window, &surface);
    
    pickPhysicalDevice
    (instance, physicalDevice, surface);
    
    createLogicalDevice
    (device, physicalDevice, graphicsQueue, presentQueue, surface, validationLayers);
    
    createSwapChain
    (device, physicalDevice, surface, swapChain, swapChainImages, swapChainImageFormat, swapChainExtent, window, windowWidth, windowHeight);
    
    createImageViews
    (device, swapChainImageViews, swapChainImages, swapChainImageFormat);
    
    createRenderPass
    (device, renderPass, swapChainImageFormat);
    
    createDescriptorSetLayout
    (device, descriptorSetLayout);
    
    createGraphicsPipeline
    (device, pipelineLayout, graphicsPipeline, renderPass, swapChainExtent, descriptorSetLayout);
    
    createFrameBuffers
    (device, swapChainFramebuffers, renderPass, swapChainImageViews, swapChainExtent);
    
    createCommandPool
    (device, physicalDevice, surface, commandPool);
    
    createTextureImage
    (physicalDevice, device, textureImage, textureImageMemory, commandPool, graphicsQueue);
    
    createTextureImageView
    (device, textureImage, textureImageView);
    
    createTextureSampler
    (device, textureSampler);
    
    createVertexBuffers
    (physicalDevice, device, vertexBuffer, vertexBufferMemory, commandPool, graphicsQueue);
    
    createIndexBuffers
    (physicalDevice, device, indexBuffer, indexBufferMemory, commandPool, graphicsQueue);
    
    createUniformBuffers
    (physicalDevice, device, uniformBuffers, uniformBuffersMemory, swapChainImages);
    
    createDescriptorPool
    (device, descriptorPool, swapChainImages);
    
    createDescriptorSets
    (device, uniformBuffers, descriptorSetLayout, descriptorPool, descriptorSets, swapChainImages, textureImageView, textureSampler);

    createCommandBuffers
    (device, commandBuffers, commandPool, graphicsPipeline, swapChainFramebuffers, renderPass, swapChainExtent, vertexBuffer, indexBuffer, pipelineLayout, descriptorSets);
    
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

void DaedalusEngine::cleanupSwapChain() {
    for (auto framebuffer : swapChainFramebuffers) {
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
    (device, renderPass, swapChainImageFormat);
    
    createGraphicsPipeline
    (device, pipelineLayout, graphicsPipeline, renderPass, swapChainExtent, descriptorSetLayout);
    
    createFrameBuffers
    (device, swapChainFramebuffers, renderPass, swapChainImageViews, swapChainExtent);
    
    createUniformBuffers
    (physicalDevice, device, uniformBuffers, uniformBuffersMemory, swapChainImages);
    
    createDescriptorPool
    (device, descriptorPool, swapChainImages);
    
    createDescriptorSets
    (device, uniformBuffers, descriptorSetLayout, descriptorPool, descriptorSets, swapChainImages, textureImageView, textureSampler);
    
    createCommandBuffers
    (device, commandBuffers, commandPool, graphicsPipeline, swapChainFramebuffers, renderPass, swapChainExtent, vertexBuffer, indexBuffer, pipelineLayout, descriptorSets);
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
