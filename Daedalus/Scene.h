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
    
    Scene(int objectCount);
    
    struct Object {
        
        Meshes::Type mesh;
        glm::vec3 lightPosition;
        glm::vec3 lightColor;
        
        //Animation animation;
        //Path path;
        
        uint32_t frameDataOffset;
        
        glm::mat4 model;
        float alpha;
        
    };
    
    const std::vector<Object> &objects() const { return objects_; }
    
    // for simulations
    //unsigned int random_seed() { return random_device_(); }
    
    void setFrameDataSize(uint32_t size);
    void update(float time, int begin, int end);
    
private:
    
    std::random_device randomDevice_;
    std::vector<Object> objects_;
    
};
