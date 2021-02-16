#include "Daedalus.h"

#include <array>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Meshes.h"
#include "Shell.h"

Daedalus::Daedalus(const std::vector<std::string> &args)
        : Engine("Daedalus", args),
        multithread_(true),
        usePushConstants_(false),
        scene_(0),
        camera_(2.5f), frameData_(),
        renderPassClearValue_({{0.0f, 0.1f, 0.2f, 1.0f}}),
        renderPassBeginInfo_(), primaryCommandBeginInfo_(),
        primaryCommandSubmitInfo_() {
    
    for (std::string arg : args) {
        
        if (arg == "-s")
            multithread_ = false;
        else if (arg == "-p")
            usePushConstants_ = true;
        
    }

            initWorkers();
            
}

Daedalus::~Daedalus() {}

void Daedalus::initWorkers() {
    
    
    
}

void Daedalus::attachShell(Shell &shell) {
    
    
    
}

void Daedalus::detachShell() {
    
    
    
}

void Daedalus::createRenderPass() {
    
    
    
}

void Daedalus::createShaderModules() {
    
    
    
}

void Daedalus::createDescriptorSetLayout() {
    
    
    
}

void Daedalus::createPipelineLayout() {
    
    
    
}

void Daedalus::createPipeline() {
    
    
    
}

void Daedalus::createFrameData(int count) {
    
    
    
}

void Daedalus::destroyFrameData() {
    
    
    
}

void Daedalus::createFences() {
    
    
    
}

void Daedalus::createCommandBuffers() {
    
    
    
}

void Daedalus::createBuffers() {
    
    
    
}

void Daedalus::createBufferMemory() {
    
    
    
}

void Daedalus::createDescriptorSets() {
    
    
    
}

void Daedalus::attachSwapchain() {
    
    
    
}

void Daedalus::detachSwapchain() {
    
    
    
}

void Daedalus::prepareViewport(const VkExtent2D &extent) {
    
    
    
}

void Daedalus::prepareFramebuffers(VkSwapchainKHR swapchain) {
    
    
    
}

void Daedalus::updateCamera() {
    
    
    
}

void Daedalus::drawObject(const Scene::Object &object, FrameData &data, VkCommandBuffer command) const {
    
    
    
}

void Daedalus::updateScene(const Worker &worker) {
    
    
    
}

void Daedalus::drawObjects(Worker &worker) {
    
    
    
}

void Daedalus::onKey(Key key) {
    
    
    
}

void Daedalus::onTick() {
    
    
    
}

void Daedalus::onFrame() {
    
    
    
}

Daedalus::Worker::Worker(Daedalus &daedalus, int index, int object_begin, int object_end)
    : daedalus_(daedalus),
    index_(index),
    objectBegin_(object_begin),
    objectEnd_(object_end),
    tickInterval_(1.0f / daedalus.settings_.ticksPerSecond),
    state_(INIT) {
    
    
    
}

void Daedalus::Worker::start() {
    
    
    
}

void Daedalus::Worker::stop() {
    
    
    
}

void Daedalus::Worker::updateScene() {
    
    
    
}

void Daedalus::Worker::drawObjects(VkFramebuffer fb) {
    
    
    
}

void Daedalus::Worker::waitIdle() {
    
    
    
}

void Daedalus::Worker::updateLoop() {
    
    
    
}
