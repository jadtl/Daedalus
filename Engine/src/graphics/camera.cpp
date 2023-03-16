#include "graphics/camera.h"

Camera::Camera(vec3 position, vec3 front, vec3 up, f32 fov):
    _position(position), _front(front), _up(up), _fov(fov)
{
}

mat4 Camera::view()
{
    return lookAt(_position, _position + _front, _up);
}

void Camera::setPosition(vec3 position)
{
    _position = position;
}

void Camera::setFront(vec3 front)
{
    _front = front;
}

void Camera::setUp(vec3 up)
{
    _up = up;
}

void Camera::setFov(f32 fov)
{
    _fov = fov;
}
