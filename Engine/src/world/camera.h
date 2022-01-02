#pragma once

#include <defines.h>

#include <math/linalg.h>

namespace ddls {
    /**
     * @brief An abstract camera class used by Daedalus
     */
    class DDLS_API Camera {
    public:
        // Default camera specs
        Camera() {}
        virtual ~Camera() = default;

        virtual void start() {}
        virtual void update() {}

        /**
         * Gets the distance of the near pane of the view frustum.
         * @return The distance of the near pane of the view frustum.
         */
        float get_near_plane() const { return near_plane; }
        void set_near_plane(float near_plane) { this->near_plane = near_plane; }

        /**
         * Gets the distance of the view frustum's far plane.
         * @return The distance of the view frustum's far plane.
         */
        float get_far_plane() const { return far_plane; }
        void set_far_plane(float far_plane) { this->far_plane = far_plane; }

        /**
         * Gets the field of view angle for the view frustum.
         * @return The field of view angle for the view frustum.
         */
        float get_field_of_view() const { return field_of_view; }
        void set_field_of_view(float field_of_view) { this->field_of_view = field_of_view; }

        const vec3& get_position() const { return position; }
        const vec3& get_rotation() const { return rotation; }
        const vec3& get_velocity() const { return velocity; }

        /**
         * Gets the view matrix created by the current camera position and rotation.
         * @return The view matrix created by the current camera position and rotation.
         */
        const mat4& get_view_matrix() const { return view_matrix; }

        /**
         * Gets the projection matrix used in the current scene render.
         * @return The projection matrix used in the current scene render.
         */
        const mat4& get_projection_matrix() const { return projection_matrix; }

    protected:
        float near_plane, far_plane;
        float field_of_view;

        vec3 position;
        vec3 rotation;
        vec3 velocity;

        mat4 view_matrix;
        mat4 projection_matrix;
    };
}