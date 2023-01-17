#include <daedalus.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <memory>

using namespace ddls;

const char *appName = "Daedalus";
const char *engineName = "Daedalus";

const u32 width = 1200;
const u32 height = 1024;

const std::vector<vk::Vertex> _vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};
VkBuffer _vertexBuffer;
VkDeviceMemory _vertexBufferMemory;
const std::vector<u16> _vertexIndices = {
    0, 1, 2, 2, 3, 0
};
VkBuffer _indexBuffer;
VkDeviceMemory _indexBufferMemory;

std::vector<VkBuffer> _uniformBuffers;
std::vector<VkDeviceMemory> _uniformBuffersMemory;
std::vector<void*> _uniformBuffersMapped;

GLFWwindow *window;
Renderer *renderer;
Gui *gui;

VkRenderPass renderPass;
std::vector<VkFramebuffer> framebuffers;

VkCommandPool _commandPool;
std::vector<VkCommandBuffer> commandBuffers;

VkDescriptorPool descriptorPool;
VkDescriptorSetLayout descriptorSetLayout;
std::vector<VkDescriptorSet> descriptorSets;
VkPipelineLayout pipelineLayout;

ddls::Pipeline *pipeline;
ddls::Pipeline *pipelineWireframe;

void transferDataToBuffer(
    void *data,
    VkBuffer &buffer,
    VkDeviceMemory &bufferMemory,
    VkDeviceSize bufferSize,
    VkBufferUsageFlagBits usage,
    VkPhysicalDevice physicalDevice,
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue)
{
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    vk::createBuffer(
        physicalDevice,
        device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer,
        stagingBufferMemory);

    void *map;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &map);
    memcpy(map, data, (u32)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    vk::createBuffer(
        physicalDevice,
        device,
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        buffer,
        bufferMemory);
    
    vk::copyBuffer(buffer, stagingBuffer, bufferSize, device, commandPool, queue);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void initializeBuffers()
{
    transferDataToBuffer(
        (void*)_vertices.data(), 
        _vertexBuffer,
        _vertexBufferMemory,
        sizeof(_vertices[0]) * _vertices.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        renderer->physicalDevice(),
        renderer->device(),
        _commandPool,
        renderer->queue());

    transferDataToBuffer(
        (void*)_vertexIndices.data(), 
        _indexBuffer,
        _indexBufferMemory,
        sizeof(_vertexIndices[0]) * _vertexIndices.size(),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        renderer->physicalDevice(),
        renderer->device(),
        _commandPool,
        renderer->queue());

    VkDeviceSize bufferSize = sizeof(vk::UniformBufferObject);

    _uniformBuffers.resize(renderer->MaxFramesInFlight());
    _uniformBuffersMemory.resize(renderer->MaxFramesInFlight());
    _uniformBuffersMapped.resize(renderer->MaxFramesInFlight());

    for (u32 i = 0; i < renderer->MaxFramesInFlight(); i++) {
        vk::createBuffer(
            renderer->physicalDevice(),
            renderer->device(),
            bufferSize, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            _uniformBuffers[i], 
            _uniformBuffersMemory[i]);

        vkMapMemory(renderer->device(), _uniformBuffersMemory[i], 0, bufferSize, 0, &_uniformBuffersMapped[i]);
    }
}

void initializeRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = renderer->swapchain()->format();
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

    Assert(vkCreateRenderPass(renderer->device(), &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS,
        "Failed to create render pass!");
}

void initializeCommands()
{
    vk::createCommandPool(
        renderer->device(),
        &_commandPool,
        renderer->queueFamilyIndex(),
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    );

    commandBuffers.resize(renderer->swapchain()->imageCount());
    vk::createCommandBuffers(
        renderer->device(),
        commandBuffers.data(),
        (u32)commandBuffers.size(),
        _commandPool
    );
}

void initializePipelineLayout()
{
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

    Assert(vkCreateDescriptorSetLayout(renderer->device(), &layoutInfo, nullptr, &descriptorSetLayout) == VK_SUCCESS,
        "Failed to create descriptor set layout!");

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<u32>(renderer->MaxFramesInFlight());

    VkDescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = &poolSize;
    descriptorPoolInfo.maxSets = static_cast<u32>(renderer->MaxFramesInFlight());

    Assert(vkCreateDescriptorPool(renderer->device(), &descriptorPoolInfo, nullptr, &descriptorPool) == VK_SUCCESS,
        "Failed to create descriptor pool!");

    std::vector<VkDescriptorSetLayout> layouts(renderer->MaxFramesInFlight(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
    descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocInfo.descriptorPool = descriptorPool;
    descriptorSetAllocInfo.descriptorSetCount = static_cast<u32>(renderer->MaxFramesInFlight());
    descriptorSetAllocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(renderer->MaxFramesInFlight());
    Assert(vkAllocateDescriptorSets(renderer->device(), &descriptorSetAllocInfo, descriptorSets.data()) == VK_SUCCESS,
        "Failed to allocate descriptor sets!");

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    Assert(vkCreatePipelineLayout(renderer->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS,
        "Failed to create pipeline layout!");
}

void initializeDescriptorSets()
{
    for (u32 i = 0; i < renderer->MaxFramesInFlight(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = _uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(vk::UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(renderer->device(), 1, &descriptorWrite, 0, nullptr);
    }
}

void updateUniformBuffer(u32 currentFrame)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    vk::UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), renderer->rotate, glm::vec3(0.0, 1.0, 0.0));
    ubo.model = glm::rotate(ubo.model, time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), (f32)renderer->swapchain()->extent().width / (f32)renderer->swapchain()->extent().height, 0.1f, 10.0f);
    // Compensate for OpenGL's Y coordinate inversion
    ubo.proj[1][1] *= -1;

    memcpy(_uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
}

void recordCommands(u32 currentFrame, u32 imageIndex, Swapchain *swapchain)
{
    VkCommandBuffer commandBuffer = commandBuffers[currentFrame];

    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    Assert(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS,
        "Failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchain->extent();
    VkClearValue clearColor = {{{renderer->red, renderer->green, renderer->blue, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<f32>(swapchain->extent().width);
    viewport.height = static_cast<f32>(swapchain->extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchain->extent();
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
        pipelineLayout, 
        0, 
        1, 
        &descriptorSets[currentFrame], 
        0, 
        nullptr);

    vkCmdDrawIndexed(commandBuffer, static_cast<u32>(_vertexIndices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    Assert(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS,
        "Failed to record command buffer");
}

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, appName, nullptr, nullptr);

    renderer = new Renderer(window, appName, engineName);
    gui = new Gui(
        window,
        renderer->instance(),
        renderer->physicalDevice(),
        renderer->device(),
        renderer->queue(),
        renderer->queueFamilyIndex(),
        renderer->swapchain(),
        false
    );

    // Render pass
    initializeRenderPass();
    framebuffers.resize(renderer->swapchain()->imageCount());
    vk::createFramebuffers(
        renderer->device(),
        framebuffers.data(),
        renderer->swapchain()->imageCount(),
        renderPass,
        renderer->swapchain()->extent(),
        renderer->swapchain()->imageViews().data()
    );
    renderer->swapchain()->addFramebuffersRecreateCallback(framebuffers.data(), renderPass);

    // Graphics pipeline
    VkShaderModule vertexShader = vk::loadShader(renderer->device(), "shader.vert.spv");
    VkShaderModule fragmentShader = vk::loadShader(renderer->device(), "shader.frag.spv");
    initializePipelineLayout();
    pipeline = new Pipeline(
        renderer->device(),
        renderer->swapchain(),
        renderPass,
        pipelineLayout,
        VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
        vertexShader,
        fragmentShader,
        VK_POLYGON_MODE_FILL
    );
    renderer->swapchain()->addPipelineRecreateCallback(pipeline);
    //pipelineWireframe = new Pipeline(
        //renderer->device(),
        //renderer->swapchain(), renderPass,
        //pipelineLayout,
        //VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        //vertexShader,
        //fragmentShader,
        //VK_POLYGON_MODE_LINE
    //);

    // Buffers and commands
    initializeCommands();
    initializeBuffers();
    initializeDescriptorSets();

    renderer->wireframe = false;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        gui->newFrame();
        ImGui::ShowDemoWindow();
        ImGui::Begin("Config");
        //ImGui::Checkbox("Wireframe", &renderer->wireframe);
        ImGui::SliderFloat("R", &renderer->red, 0.0, 1.0f);
        ImGui::SliderFloat("G", &renderer->green, 0.0, 1.0f);
        ImGui::SliderFloat("B", &renderer->blue, 0.0, 1.0f);
        ImGui::SliderFloat("Rotate", &renderer->rotate, 0.0, (f32)glm::radians(90.0));
        ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
        gui->render();

        u32 imageIndex = renderer->newFrame();
        if (imageIndex == (u32)-1) continue;
        gui->recordCommands(renderer->currentFrame(), imageIndex, renderer->swapchain());
        recordCommands(renderer->currentFrame(), imageIndex, renderer->swapchain());
        renderer->submit(commandBuffers[renderer->currentFrame()]);
        renderer->submit(gui->commandBuffers()[renderer->currentFrame()]);
        updateUniformBuffer(renderer->currentFrame());
        renderer->render(imageIndex);
    }

    vkDeviceWaitIdle(renderer->device());

    vkDestroyShaderModule(renderer->device(), vertexShader, nullptr);
    vkDestroyShaderModule(renderer->device(), fragmentShader, nullptr);

    vkDestroyBuffer(renderer->device(), _vertexBuffer, nullptr);
    vkFreeMemory(renderer->device(), _vertexBufferMemory, nullptr);

    vkDestroyBuffer(renderer->device(), _indexBuffer, nullptr);
    vkFreeMemory(renderer->device(), _indexBufferMemory, nullptr);

    for (u32 i = 0; i < renderer->MaxFramesInFlight(); i++) {
        vkDestroyBuffer(renderer->device(), _uniformBuffers[i], nullptr);
        vkFreeMemory(renderer->device(), _uniformBuffersMemory[i], nullptr);
    }

    vkDestroyCommandPool(renderer->device(), _commandPool, nullptr);

    vkDestroyDescriptorSetLayout(renderer->device(), descriptorSetLayout, nullptr);
    vkDestroyPipelineLayout(renderer->device(), pipelineLayout, nullptr);

    vkDestroyRenderPass(renderer->device(), renderPass, nullptr);


    vkDestroyDescriptorPool(renderer->device(), descriptorPool, nullptr);

    for (auto framebuffer: framebuffers) vkDestroyFramebuffer(renderer->device(), framebuffer, nullptr);

    delete pipeline;
    delete pipelineWireframe;
    delete gui;
    delete renderer;

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}