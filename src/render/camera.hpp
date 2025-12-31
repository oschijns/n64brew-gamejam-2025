#pragma once

#include <t3d/t3d.h>
#include "math/vec3.hpp"

namespace jam::render
{
    /// @brief Define a light in the scene
    class Camera
    {
    protected:
        /// @brief Viewport
        T3DViewport viewport = t3d_viewport_create();

    public:
        /// @brief Position of the camera
        Vec3 origin = Vec3(0.f, 12.f, 12.f);

        /// @brief Target of the camera
        Vec3 target;

        /// @brief Up direction
        Vec3 up = Vec3(0.f, 1.f, 0.f);

        /// @brief Field of view angle in radiants
        real fov_angle_rad = T3D_DEG_TO_RAD(65.f);

        /// @brief Near plane of the projection
        real plane_near = 10.f;

        /// @brief Far plane of the projection
        real plane_far = 1000.f;


    public:
        /// @brief Default camera position
        constexpr inline Camera() = default;

        /// @brief Set the camera position 
        /// @param origin_ Position of the camera
        /// @param target_ Target   of the camera
        constexpr inline Camera(
            const Vec3 & origin_,
            const Vec3 & target_,
            const Vec3 & up_,
            real fov_angle_rad_,
            real plane_near_,
            real plane_far_
        ):
            origin(origin_),
            target(target_),
            up(up_),
            fov_angle_rad(fov_angle_rad_),
            plane_near(plane_near_),
            plane_far(plane_far_)
        {}

        /// @brief Copy constructor
        Camera(const Camera & other) = delete;

        /// @brief Copy operator
        void operator=(const Camera & other) = delete;


    public:

        /// @brief Update the projection matrix of the viewport
        inline void update_projection()
        {
            t3d_viewport_set_projection(&viewport,
                fov_angle_rad, plane_near, plane_far);
        }

        /// @brief Update the position and target of the viewport
        inline void update_look_at()
        {
            t3d_viewport_look_at(&viewport, 
                &origin.to_t3d(), &target.to_t3d(), &up.to_t3d());
        }
    };

}