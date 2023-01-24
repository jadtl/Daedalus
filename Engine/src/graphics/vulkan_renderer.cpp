#include "graphics/vulkan_renderer.h"

#include "core/log.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

#include <set>
#include <algorithm>
#include <fstream>
#include <chrono>

namespace ddls {
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    ignore(messageSeverity, messageType, pUserData);

    Log::Out(Log::Styles::Default, Log::Colours::LightBlue, "VALIDATION", pCallbackData->pMessage);

    return VK_FALSE;
}

static void framebufferResizeCallback(GLFWwindow *window, i32 width, i32 height)
{
    ignore(width, height);

    auto renderer = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));

    renderer->setFramebufferResized();
}

VulkanRenderer::VulkanRenderer(
    GLFWwindow *window, 
    const char* appName, 
    const char* engineName) : _window(window)
{
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

    Assert(checkValidationLayerSupport(_validationLayers) || !_enableValidationLayers, 
        "Validations layers requested but not supported!");

    // Fill application info
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = appName;
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = engineName;
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_0;

    // Fill instance create info
    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    std::vector<const char*> requiredExtensions = 
        getRequiredExtensions(&instanceCreateInfo, _enableValidationLayers);
    instanceCreateInfo.enabledExtensionCount = (uint32_t)requiredExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();
    if (_enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        fillDebugMessengerCreateInfo(debugCreateInfo, debugCallback);
        instanceCreateInfo.pNext = &debugCreateInfo;
        instanceCreateInfo.enabledLayerCount = (uint32_t)_validationLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = _validationLayers.data();
    }
    else
    {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = nullptr;
    }

    enumerateAvailableExtensions();

    Assert(vkCreateInstance(&instanceCreateInfo, nullptr, &_instance) == VK_SUCCESS, 
        "Instance creation failed!");

#ifdef DDLS_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    fillDebugMessengerCreateInfo(debugCreateInfo, debugCallback);

    auto createFunction = (PFN_vkCreateDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
    Assert(createFunction, 
        "Failed to get vkCreateDebugUtilsMessengerEXT function!");
    Assert(createFunction(_instance, &debugCreateInfo, nullptr, &_debugMessenger) == VK_SUCCESS,
        "Failed to create debug messenger!");
#endif

    Assert(glfwCreateWindowSurface(_instance, window, nullptr, &_surface) == VK_SUCCESS,
        "Failed to create window surface!");

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

    Assert(deviceCount, 
        "No physical device was found!");

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());

    // Find a suitable physical device
    for (const auto& physicalDevice: physicalDevices)
    {
        if (isDeviceSuitable(physicalDevice))
        {
            _physicalDevice = physicalDevice;
            break;
        }
    }

    Assert(_physicalDevice != VK_NULL_HANDLE, 
        "No suitable physical device was found!");

    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(_physicalDevice, &physicalDeviceProperties);

    Log::Info("Picking physical device ", physicalDeviceProperties.deviceName);

    u32 extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, extensions.data());
    Log::Info("Device extensions");
    for (const auto& extension : extensions)
        Log::Info('\t', extension.extensionName);

    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = (u32)queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = 0;
#ifdef DDLS_PLATFORM_MACOS
    std::vector<const char*> enabledExtensions;
    _deviceExtensions.push_back("VK_KHR_portability_subset");
#endif
    deviceCreateInfo.enabledExtensionCount = (u32)_deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = _deviceExtensions.data();
    if (_enableValidationLayers) {
        deviceCreateInfo.enabledLayerCount = (u32)_validationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = _validationLayers.data();
    } else {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    Assert(vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device) == VK_SUCCESS,
        "Failed to create device!");

    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);

    createSwapchain();

    createSwapchainImageViews();

    auto vertShaderCode = readFile("shader.vert.spv");
    auto fragShaderCode = readFile("shader.frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = (u32)attributeDescriptions.size();
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)_swapchainExtent.width;
    viewport.height = (f32)_swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapchainExtent;

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // Counter-clockwise because of Y-flip in projection matrix
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = _swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    Assert(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) == VK_SUCCESS,
        "Failed to create render pass!");

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    Assert(vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_descriptorSetLayout) == VK_SUCCESS,
        "Failed to create descriptor set layout!");

    // Initialize ImGui Vulkan structures
    createRenderPassImGui();
    createDescriptorPoolImGui();
    createCommandsImGui();
    createFramebuffersImGui();

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<u32>(_MaxFramesInFlight);

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = &poolSize;
    descriptorPoolInfo.maxSets = static_cast<u32>(_MaxFramesInFlight);

    Assert(vkCreateDescriptorPool(_device, &descriptorPoolInfo, nullptr, &_descriptorPool) == VK_SUCCESS,
        "Failed to create descriptor pool!");

    std::vector<VkDescriptorSetLayout> layouts(_MaxFramesInFlight, _descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = _descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<u32>(_MaxFramesInFlight);
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    _descriptorSets.resize(_MaxFramesInFlight);
    Assert(vkAllocateDescriptorSets(_device, &descriptorSetAllocInfo, _descriptorSets.data()) == VK_SUCCESS,
        "Failed to allocate descriptor sets!");

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    Assert(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout) == VK_SUCCESS,
        "Failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = _pipelineLayout;
    pipelineInfo.renderPass = _renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    Assert(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline) == VK_SUCCESS,
        "Failed to create graphics pipeline!");

    rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
    pipelineInfo.pRasterizationState = &rasterizer;

    Assert(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipelineWireframe) == VK_SUCCESS,
        "Failed to create graphics pipeline!");

    vkDestroyShaderModule(_device, vertShaderModule, nullptr);
    vkDestroyShaderModule(_device, fragShaderModule, nullptr);

    createSwapchainFramebuffers();

    indices = findQueueFamilies(_physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

    Assert(vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool) == VK_SUCCESS,
        "Failed to create command pool!");

    VkDeviceSize bufferSize = sizeof(_vertices[0]) * _vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *data;
    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _vertices.data(), (u32)bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _vertexBuffer,
        _vertexBufferMemory);
    
    copyBuffer(_vertexBuffer, stagingBuffer, bufferSize);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

    bufferSize = sizeof(_indices[0]) * _indices.size();
    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    vkMapMemory(_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, _indices.data(), (u32)bufferSize);
    vkUnmapMemory(_device, stagingBufferMemory);

    createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        _indexBuffer,
        _indexBufferMemory);

    copyBuffer(_indexBuffer, stagingBuffer, bufferSize);

    vkDestroyBuffer(_device, stagingBuffer, nullptr);
    vkFreeMemory(_device, stagingBufferMemory, nullptr);

    bufferSize = sizeof(UniformBufferObject);

    _uniformBuffers.resize(_MaxFramesInFlight);
    _uniformBuffersMemory.resize(_MaxFramesInFlight);
    _uniformBuffersMapped.resize(_MaxFramesInFlight);

    for (u32 i = 0; i < _MaxFramesInFlight; i++) {
        createBuffer(
            bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            _uniformBuffers[i], 
            _uniformBuffersMemory[i]);

        vkMapMemory(_device, _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
    }

    _commandBuffers.resize(_MaxFramesInFlight);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = _commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (u32)_commandBuffers.size();

    Assert(vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()) == VK_SUCCESS,
        "Failed to allocate command buffers!");

    for (u32 i = 0; i < _MaxFramesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = _descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(_device, 1, &descriptorWrite, 0, nullptr);
    }

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Need this workaround to avoid a deadlock at the first frame
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    _imageAvailableSemaphores.resize(_MaxFramesInFlight);
    _renderFinishedSemaphores.resize(_MaxFramesInFlight);
    _inFlightFences.resize(_MaxFramesInFlight);
    for (u32 i = 0; i < _MaxFramesInFlight; i++)
    {
        Assert(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]) == VK_SUCCESS,
            "Failed to create semaphore!");

        Assert(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]) == VK_SUCCESS,
            "Failed to create semaphore!");

        Assert(vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]) == VK_SUCCESS,
            "Failed to create fence!");
    }

    _currentFrame = 0;
}

VulkanRenderer::~VulkanRenderer()
{
    destroySwapchain();

    for (u32 i = 0; i < _MaxFramesInFlight; i++) {
        vkDestroyBuffer(_device, _uniformBuffers[i], nullptr);
        vkFreeMemory(_device, _uniformBuffersMemory[i], nullptr);
    }

    vkDestroyPipeline(_device, _graphicsPipelineWireframe, nullptr);

    for (auto framebuffer : _framebuffersImGui) vkDestroyFramebuffer(_device, framebuffer, nullptr);
    vkDestroyRenderPass(_device, _renderPassImGui, nullptr);
    vkDestroyCommandPool(_device, _commandPoolImGui, nullptr);
    vkDestroyDescriptorPool(_device, _descriptorPoolImGui, nullptr);

    vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(_device, _descriptorSetLayout, nullptr);

    vkDestroyBuffer(_device, _vertexBuffer, nullptr);
    vkFreeMemory(_device, _vertexBufferMemory, nullptr);

    vkDestroyBuffer(_device, _indexBuffer, nullptr);
    vkFreeMemory(_device, _indexBufferMemory, nullptr);

    for (u32 i = 0; i < _MaxFramesInFlight; i++)
    {
        vkDestroySemaphore(_device, _imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(_device, _renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(_device, _inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(_device, _commandPool, nullptr);

    vkDestroyPipeline(_device, _graphicsPipeline, nullptr);

    vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);

    vkDestroyRenderPass(_device, _renderPass, nullptr);

    vkDestroyDevice(_device, nullptr);

#ifdef DDLS_DEBUG
    auto destroyFunction = (PFN_vkDestroyDebugUtilsMessengerEXT)
        vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");
    destroyFunction(_instance, _debugMessenger, nullptr);
#endif

    vkDestroySurfaceKHR(_instance, _surface, nullptr);

    vkDestroyInstance(_instance, nullptr);
}

void VulkanRenderer::render(ImDrawData *drawData)
{
    vkWaitForFences(_device, 1, &_inFlightFences[_currentFrame], VK_TRUE, (u64)-1);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(_device, _swapchain, (u64)-1, _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        _framebufferResized = false;
        recreateSwapchain();
        return;
    }

    Assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR,
        "Failed to acquire swapchain image!");

    vkResetFences(_device, 1, &_inFlightFences[_currentFrame]);

    vkResetCommandBuffer(_commandBuffers[_currentFrame], 0);

    recordCommandBuffer(_commandBuffers[_currentFrame], imageIndex);

    updateUniformBuffer(_currentFrame);

    vkResetCommandBuffer(_commandBuffersImGui[_currentFrame], 0);

    recordImGui(_commandBuffersImGui[_currentFrame], imageIndex, drawData);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    std::array<VkCommandBuffer, 2> submitCommandBuffers = 
        { _commandBuffers[_currentFrame], _commandBuffersImGui[_currentFrame] };
    VkSemaphore waitSemaphores[] = {_imageAvailableSemaphores[_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = (u32)submitCommandBuffers.size();
    submitInfo.pCommandBuffers = submitCommandBuffers.data();
    VkSemaphore signalSemaphores[] = {_renderFinishedSemaphores[_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    Assert(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[_currentFrame]) == VK_SUCCESS,
        "Failed to submit draw command buffer!");

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = {_swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        _framebufferResized = false;
        recreateSwapchain();
    }
    else
    {
        Assert(result == VK_SUCCESS,
            "Failed to present swapchain image!");
    }

    _currentFrame = (_currentFrame + 1) % _MaxFramesInFlight;
}

void VulkanRenderer::recreateSwapchain()
{
    i32 width = 0, height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(_window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(_device);

    destroySwapchain();

    createSwapchain();
    createSwapchainImageViews();
    createSwapchainFramebuffers();
    for (auto framebuffer: _framebuffersImGui) vkDestroyFramebuffer(_device, framebuffer, nullptr);
    createFramebuffersImGui();
}

void VulkanRenderer::createSwapchain()
{
    swapchainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // Adding one reduces internal waiting
    u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = _surface;
    swapchainCreateInfo.minImageCount = imageCount;
    swapchainCreateInfo.imageFormat = surfaceFormat.format;
    swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapchainCreateInfo.imageExtent = extent;
    // Always 1 unless developing a stereoscoping 3D application
    swapchainCreateInfo.imageArrayLayers = 1;
    // Rendering directly to the images, i.e. not doing any post-processing
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
    u32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily)
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } 
    else
    {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount = 0;
        swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    }
    // No pre-transform
    swapchainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    // Ignore the alpha channel
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = presentMode;
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    Assert(vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &_swapchain) == VK_SUCCESS,
        "Failed to create swapchain!");

    vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, nullptr);
    _swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapchain, &imageCount, _swapchainImages.data());

    _swapchainImageFormat = surfaceFormat.format;
    _swapchainExtent = extent;
}

void VulkanRenderer::createSwapchainImageViews()
{
    _swapchainImageViews.resize(_swapchainImages.size());

    for (u32 i = 0; i < _swapchainImages.size(); i++)
    {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = _swapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = _swapchainImageFormat;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        Assert(vkCreateImageView(_device, &imageViewCreateInfo, nullptr, &_swapchainImageViews[i]) == VK_SUCCESS,
            "Failed to create swapchain image view!");
    }
}

void VulkanRenderer::createSwapchainFramebuffers()
{
    _swapchainFramebuffers.resize(_swapchainImageViews.size());

    for (size_t i = 0; i < _swapchainImageViews.size(); i++) {
        VkImageView attachments[] = {
            _swapchainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = _renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = _swapchainExtent.width;
        framebufferInfo.height = _swapchainExtent.height;
        framebufferInfo.layers = 1;

        Assert(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapchainFramebuffers[i]) == VK_SUCCESS,
            "Failed to create framebuffer!");
    }
}

void VulkanRenderer::destroySwapchain()
{
    for (size_t i = 0; i < _swapchainFramebuffers.size(); i++)
        vkDestroyFramebuffer(_device, _swapchainFramebuffers[i], nullptr);

    for (size_t i = 0; i < _swapchainImageViews.size(); i++)
        vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);

    vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

std::vector<const char*> VulkanRenderer::getRequiredExtensions(
    VkInstanceCreateInfo* instanceCreateInfo, 
    bool enableValidationLayers)
{
    // Get required extensions for windowing
    u32 glfwExtensionCount = 0;
    const char **glfwExtensions = 
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> requiredExtensions;
    for (u32 i = 0; i < glfwExtensionCount; i++)
        requiredExtensions.push_back(glfwExtensions[i]);

    // Add portability enumeration extension when on Apple platform
#ifdef __APPLE__
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    requiredExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    instanceCreateInfo->flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
    ignore(instanceCreateInfo);
#endif

    // Add debug utils if validation layers are requested
    if (enableValidationLayers)
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    return requiredExtensions;
}

void VulkanRenderer::enumerateAvailableExtensions()
{
    u32 extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    Log::Info("Available extensions");
    for (const auto& extension : extensions)
        Log::Info('\t', extension.extensionName);
}

bool VulkanRenderer::checkValidationLayerSupport(
    std::vector<const char*> validationLayers)
{
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName: validationLayers)
    {
        bool layerFound = false;
        for (const auto& layerProperties: availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) return false;
    }

    return true;
}

void VulkanRenderer::fillDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo, 
    PFN_vkDebugUtilsMessengerCallbackEXT debugCallback)
{
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.flags = 0;
    createInfo.pUserData = nullptr;
}

bool VulkanRenderer::isDeviceSuitable(
    VkPhysicalDevice physicalDevice)
{
    VkPhysicalDeviceProperties physicalDeviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
    Log::Info("Checking if device ", physicalDeviceProperties.deviceName, " is suitable");
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

    VulkanRenderer::QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        swapchainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

VulkanRenderer::QueueFamilyIndices VulkanRenderer::findQueueFamilies(
    VkPhysicalDevice physicalDevice)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    // Finding a queue that supports graphics commands
    VulkanRenderer::QueueFamilyIndices indices;
    u32 i = 0;
    for (const auto& queueFamily: queueFamilies)
    {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, _surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.isComplete()) break;

        i++;
    }

    return indices;
}

bool VulkanRenderer::checkDeviceExtensionSupport(
    VkPhysicalDevice physicalDevice)
{
    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

VulkanRenderer::swapchainSupportDetails VulkanRenderer::querySwapChainSupport(
    VkPhysicalDevice physicalDevice)
{
    swapchainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, _surface, &details.capabilities);

    u32 formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, _surface, &formatCount, details.formats.data());
    }

    u32 presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, _surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR VulkanRenderer::chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;

    int width, height;
    glfwGetFramebufferSize(_window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

std::vector<char> ddls::VulkanRenderer::readFile(
    const std::string& fileName)
{
    // Start reading at the end of the file and avoid text transformations
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    Assert(file.is_open(),
        fmt::format("Failed to open file {}!", fileName));

    u32 fileSize = (u32)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

VkShaderModule VulkanRenderer::createShaderModule(
    std::vector<char> code)
{
    VkShaderModuleCreateInfo shaderCreateInfo{};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = (u32)code.size();
    shaderCreateInfo.pCode = reinterpret_cast<const u32*>(code.data());

    VkShaderModule shaderModule;
    Assert(vkCreateShaderModule(_device, &shaderCreateInfo, nullptr, &shaderModule) == VK_SUCCESS,
        "Failed to create shader module!");
    return shaderModule;
}

u32 VulkanRenderer::findMemoryType(
    u32 typeFilter,
    VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) 
            && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    Assert(false, "Failed to find suitable memory type!");
    return (u32)-1;
}

void VulkanRenderer::copyBuffer(
    VkBuffer dstBuffer, 
    VkBuffer srcBuffer, 
    VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = _commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    // Using the graphics queue for transfers instead of a separate queue
    vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(_graphicsQueue);

    vkFreeCommandBuffers(_device, _commandPool, 1, &commandBuffer);
}

void VulkanRenderer::updateUniformBuffer(u32 currentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), rotate, glm::vec3(0.0, 1.0, 0.0));
    ubo.model = glm::rotate(ubo.model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), (f32)_swapchainExtent.width / (f32)_swapchainExtent.height, 0.1f, 10.0f);
    // Compensate for OpenGL's Y coordinate inversion
    ubo.proj[1][1] *= -1;

    memcpy(_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void VulkanRenderer::createDescriptorPoolImGui()
{
    VkDescriptorPoolSize poolSizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
    pool_info.poolSizeCount = (u32)IM_ARRAYSIZE(poolSizes);
    pool_info.pPoolSizes = poolSizes;
    Assert(vkCreateDescriptorPool(_device, &pool_info, nullptr, &_descriptorPoolImGui) == VK_SUCCESS,
        "Failed to create ImGui descriptor pool!");
}

void VulkanRenderer::createCommandsImGui()
{
    createCommandPool(&_commandPoolImGui, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    _commandBuffersImGui.resize(_swapchainImageViews.size());
    createCommandBuffers(_commandBuffersImGui.data(), static_cast<u32>(_commandBuffersImGui.size()), _commandPoolImGui);
}

void VulkanRenderer::createRenderPassImGui()
{
    VkAttachmentDescription attachment = {};
    attachment.format = _swapchainImageFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment = {};
    color_attachment.attachment = 0;
    color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    info.attachmentCount = 1;
    info.pAttachments = &attachment;
    info.subpassCount = 1;
    info.pSubpasses = &subpass;
    info.dependencyCount = 1;
    info.pDependencies = &dependency;

    Assert(vkCreateRenderPass(_device, &info, nullptr, &_renderPassImGui) == VK_SUCCESS,
        "Failed to create ImGui render pass!");
}

void VulkanRenderer::createFramebuffersImGui()
{
    VkImageView attachment[1];
    VkFramebufferCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = _renderPassImGui;
    info.attachmentCount = 1;
    info.pAttachments = attachment;
    info.width = _swapchainExtent.width;
    info.height = _swapchainExtent.height;
    info.layers = 1;

    _framebuffersImGui.resize(_swapchainImageViews.size());
    for (u32 i = 0; i < _framebuffersImGui.size(); i++)
    {
        attachment[0] = _swapchainImageViews[i];
        Assert(vkCreateFramebuffer(_device, &info, nullptr, &_framebuffersImGui[i]) == VK_SUCCESS,
            "Failed to create ImGui framebuffer!");
    }
}

void VulkanRenderer::createCommandPool(VkCommandPool* commandPool, VkCommandPoolCreateFlags flags)
{
    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    commandPoolCreateInfo.flags = flags;

    Assert(vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, commandPool) == VK_SUCCESS,
        "Failed to create ImGui command pool!");
}

void VulkanRenderer::createCommandBuffers(VkCommandBuffer* commandBuffer, u32 commandBufferCount, VkCommandPool& commandPool)
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.commandBufferCount = commandBufferCount;
    vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, commandBuffer);
}

void VulkanRenderer::createBuffer(
    VkDeviceSize size, 
    VkBufferUsageFlags usage, 
    VkMemoryPropertyFlags properties,
    VkBuffer& buffer, 
    VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    Assert(vkCreateBuffer(_device, &bufferInfo, nullptr, &buffer) == VK_SUCCESS,
        "Failed to create vertex buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(_device, buffer, &memRequirements);

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = memRequirements.size;
    memAllocInfo.memoryTypeIndex = findMemoryType(
        memRequirements.memoryTypeBits, 
        properties);

    Assert(vkAllocateMemory(_device, &memAllocInfo, nullptr, &bufferMemory) == VK_SUCCESS,
        "Failed to allocate vertex buffer memory!");

    vkBindBufferMemory(_device, buffer, bufferMemory, 0);
}

void VulkanRenderer::recordCommandBuffer(
    VkCommandBuffer commandBuffer, 
    u32 imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    Assert(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS,
        "Failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPass;
    renderPassInfo.framebuffer = _swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapchainExtent;
    VkClearValue clearColor = {{{red, green, blue, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, wireframe ? _graphicsPipelineWireframe : _graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(_swapchainExtent.width);
    viewport.height = static_cast<f32>(_swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    
    // Buffers bindings
    vkCmdBindVertexBuffers(commandBuffer, 
        0, 
        1, 
        vertexBuffers, 
        offsets);
    vkCmdBindIndexBuffer(commandBuffer, 
        _indexBuffer, 
        0, 
        VK_INDEX_TYPE_UINT16);
    vkCmdBindDescriptorSets(commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        _pipelineLayout, 
        0, 
        1, 
        &_descriptorSets[_currentFrame], 
        0, 
        nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<u32>(_indices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    Assert(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS,
        "Failed to record command buffer");
}

void VulkanRenderer::recordImGui(
    VkCommandBuffer commandBuffer, 
    u32 imageIndex, 
    ImDrawData *drawData)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    Assert(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS,
        "Failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = _renderPassImGui;
    renderPassInfo.framebuffer = _framebuffersImGui[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = _swapchainExtent;
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(_swapchainExtent.width);
    viewport.height = static_cast<f32>(_swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = _swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    Assert(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS,
        "Failed to record command buffer");
}

}
