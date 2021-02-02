#pragma once

#include <memory>
#include <random>
#include <vector>

#include <glm/glm.hpp>

#include "Meshes.h"

//class Animation
//class Path

class Scene {
    
public:
    
    Scene(int object_count);
    
    struct Object {
        
        Meshes::Type mesh;
        glm::vec3 light_position;
        glm::vec3 light_color;
        
        //Animation animation;
        //Path path;
        
        uint32_t frame_data_offset;
        
        glm::mat4 model;
        float alpha;
        
    };
    
    const std::vector<Object> &objects() const { return objects_; }
    
    // for simulations
    //unsigned int random_seed() { return random_device_(); }
    
    void set_frame_date_size(uint32_t size);
    void update(float time, int begin, int end);
    
private:
    
    std::random_device random_device_;
    std::vector<Object> objects_;
    
};
