#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.hpp>

#include <iostream>

class Shell;

class Engine {
    
public:
    
    Engine(const Engine &engine) = delete;
    Engine &operator = (const Engine &engine) = delete;
    virtual ~Engine() {}
    
    struct Settings {
        
        std::string name;
        VkExtent2D initialExtent;
        int queueCount;
        int backBufferCount;
        int ticksPerSecond;
        bool vsync;
        bool animate;

        bool validate;
        bool validateVerbose;

        bool noTick;
        bool noRender;
        bool noPresent;
        
    };
    
    const Settings &settings() const { return settings_; }

    virtual void attachShell(Shell &shell) { shell_ = &shell; }
    virtual void detachShell() { shell_ = nullptr; }

    virtual void attachSwapchain() {}
    virtual void detachSwapchain() {}
    
    enum Key {
        KEY_UNKNOWN,
        KEY_ESC,
        KEY_UP,
        KEY_DOWN,
        KEY_SPACE,
        KEY_F
    };
    
    virtual void onKey(Key key) {}
    virtual void onTick() {}
    virtual void onFrame() {}
    
protected:
    
    Engine(const std::string &name, const std::vector<std::string> &args) : settings_(), shell_(nullptr) {
        
        settings_.name = name;
        settings_.initialExtent.width = 1280;
        settings_.initialExtent.height = 1024;
        settings_.queueCount = 1;
        settings_.backBufferCount = 1;
        settings_.ticksPerSecond = 30;
        settings_.vsync = true;
        settings_.animate = true;

        settings_.validate = false;
        settings_.validateVerbose = false;

        settings_.noTick = false;
        settings_.noRender = false;
        settings_.noPresent = false;
        
        parse_args(args);
    }
    
    Settings settings_;
    Shell *shell_;
    
private:
    
    void parse_args(const std::vector<std::string> &args) {
        
        for (std::string arg : args) {
            
            if (arg == "-w") {
                
                settings_.initialExtent.width = std::stoi(arg);
                
            } else if (arg == "-h") {
                
                settings_.initialExtent.height = std::stoi(arg);
                
            } else if (arg == "-v") {
                
                #if __APPLE__
                    #include <TargetConditionals.h>
                    #if !(TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
                        settings_.validate = true;
                    #endif
                #else
                    settings_.validate = true;
                #endif
                
            } else if (arg == "-vv") {
                
                #if __APPLE__
                    #include <TargetConditionals.h>
                    #if !(TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
                        settings_.validate = true;
                        settings_.validateVerbose = true;
                    #endif
                #else
                    settings_.validate = true;
                    settings_.validate_verbose = true;
                #endif
                
            }
            
        }
        
    }

};
