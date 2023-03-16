#pragma once

#include "core/defines.h"
#include "core/types.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

/**
 * @brief A generic camera system
 * 
 */
class DDLS_API Camera
{
public:
    explicit Camera(
        vec3 position = vec3(0.0f, 0.0f, 0.0f), 
        vec3 front = vec3(0.0f, 0.0f, -1.0f), 
        vec3 up = vec3(0.0f, 1.0f, 0.0f), 
        f32 fov = 45.0f
    );

    /**
     * @brief The matrix of the camera view transformation
     * 
     * @return mat4 The view matrix
     */
    mat4 view();

    vec3 position() const { return _position; }
    vec3 front() const { return _front; }
    vec3 up() const { return _up; }
    f32 fov() const { return _fov; }

    void setPosition(vec3 position);
    void setFront(vec3 front);
    void setUp(vec3 up);
    void setFov(f32 fov);

private:
    vec3 _position;
    vec3 _front;
    vec3 _up;
    f32 _fov;
};