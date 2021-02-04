#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

class Shell;

class Engine {
    
public:
    
    Engine(const Engine &engine) = delete;
    Engine &operator = (const Engine &engine) = delete;
    virtual ~Engine() {}
    
    struct Settings {
        
        std::string name;
        VkExtent2D initial_extent;
        int queue_count;
        int back_buffer_count;
        int ticks_per_second;
        bool vsync;
        bool animate;

        bool validate;
        bool validate_verbose;

        bool no_tick;
        bool no_render;
        bool no_present;
        
    };
    
    const Settings &settings() const { return settings_; }

    virtual void attach_shell(Shell &shell) { shell_ = &shell; }
    virtual void detach_shell() { shell_ = nullptr; }

    virtual void attach_swapchain() {}
    virtual void detach_swapchain() {}
    
    enum Key {
        KEY_UNKNOWN,
        KEY_ESC,
        KEY_UP,
        KEY_DOWN,
        KEY_SPACE,
        KEY_F
    };
    
    virtual void on_key(Key key) {}
    virtual void on_tick() {}
    virtual void on_frame() {}
    
protected:
    
    Engine(const std::string &name, const std::vector<std::string> &args) : settings_(), shell_(nullptr) {
        
        settings_.name = name;
        settings_.initial_extent.width = 1280;
        settings_.initial_extent.height = 1024;
        settings_.queue_count = 1;
        settings_.back_buffer_count = 1;
        settings_.ticks_per_second = 30;
        settings_.vsync = true;
        settings_.animate = true;

        settings_.validate = false;
        settings_.validate_verbose = false;

        settings_.no_tick = false;
        settings_.no_render = false;
        settings_.no_present = false;
        
    }
    
    Settings settings_;
    Shell *shell_;
    
private:
    
    void parse_args(const std::vector<std::string> &args) {
        
        for (auto iterator = args.begin(); iterator != args.end(); ++iterator) {
            
            if (*iterator == "-w") {
                ++iterator;
                settings_.initial_extent.width = std::stoi(*iterator);
            } else if (*iterator == "-h") {
                ++iterator;
                settings_.initial_extent.height = std::stoi(*iterator);
            } else if (*iterator == "-v") {
                ++iterator;
                settings_.validate = true;
            }
            
        }
        
    }

};
