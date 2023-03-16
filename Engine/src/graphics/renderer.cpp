#include "graphics/renderer.h"

namespace ddls {

void Renderer::projectionUpdate()
{
    switch(_config.projectionType)
    {
        case Orthographic:
            _projection = ortho(0.0f, (f32)_config.width, (f32)_config.height, 0.0f, -1.0f, 1.0f);
            break;
        case Perspective:
            _projection = perspective(camera.fov(), (f32)_config.width/_config.height, _config.planeNear, _config.planeFar);
            break;
    }
}

} // namespace ddls