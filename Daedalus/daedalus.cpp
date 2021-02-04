#include "Daedalus.h"

#include <array>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Meshes.h"
#include "Shell.h"

Daedalus::Daedalus(const std::vector<std::string> &args)
        : Engine("Daedalus", args),
        multithread_(true),
        use_push_constants_(false),
        scene_(0),
        camera_(2.5f), frame_data_(),
        render_pass_clear_value_({{0.0f, 0.1f, 0.2f, 1.0f}}),
        render_pass_begin_info_(), primary_command_begin_info_(),
        primary_command_submit_info_() {
    
    for (auto it = args.begin(); it != args.end(); ++it) {
        
        if (*it == "-s")
            multithread_ = false;
        else if (*it == "-p")
            use_push_constants_ = true;
        
    }

    init_workers();
            
}

Daedalus::~Daedalus() {}

void Daedalus::init_workers() {
    
    
    
}

void Daedalus::attach_shell(Shell &shell) {
    
    
    
}

void Daedalus::detach_shell() {
    
    
    
}

void Daedalus::create_render_pass() {
    
    
    
}

void Daedalus::create_shader_modules() {
    
    
    
}

void Daedalus::create_descriptor_set_layout() {
    
    
    
}

void Daedalus::create_pipeline_layout() {
    
    
    
}

void Daedalus::create_pipeline() {
    
    
    
}

void Daedalus::create_frame_data(int count) {
    
    
    
}

void Daedalus::destroy_frame_data() {
    
    
    
}

void Daedalus::create_fences() {
    
    
    
}

void Daedalus::create_command_buffers() {
    
    
    
}

void Daedalus::create_buffers() {
    
    
    
}

void Daedalus::create_buffer_memory() {
    
    
    
}

void Daedalus::create_descriptor_sets() {
    
    
    
}

void Daedalus::attach_swapchain() {
    
    
    
}

void Daedalus::detach_swapchain() {
    
    
    
}

void Daedalus::prepare_viewport(const VkExtent2D &extent) {
    
    
    
}

void Daedalus::prepare_framebuffers(VkSwapchainKHR swapchain) {
    
    
    
}

void Daedalus::update_camera() {
    
    
    
}

void Daedalus::draw_object(const Scene::Object &object, FrameData &data, VkCommandBuffer command) const {
    
    
    
}

void Daedalus::update_scene(const Worker &worker) {
    
    
    
}

void Daedalus::draw_objects(Worker &worker) {
    
    
    
}

void Daedalus::on_key(Key key) {
    
    
    
}

void Daedalus::on_tick() {
    
    
    
}

void Daedalus::on_frame() {
    
    
    
}

Daedalus::Worker::Worker(Daedalus &daedalus, int index, int object_begin, int object_end)
    : daedalus_(daedalus),
    index_(index),
    object_begin_(object_begin),
    object_end_(object_end),
    tick_interval_(1.0f / daedalus.settings_.ticks_per_second),
    state_(INIT) {
    
    
    
}

void Daedalus::Worker::start() {
    
    
    
}

void Daedalus::Worker::stop() {
    
    
    
}

void Daedalus::Worker::update_scene() {
    
    
    
}

void Daedalus::Worker::draw_objects(VkFramebuffer fb) {
    
    
    
}

void Daedalus::Worker::wait_idle() {
    
    
    
}

void Daedalus::Worker::update_loop() {
    
    
    
}
