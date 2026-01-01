#pragma once


#include "math/base.hpp"
#include "math/interpolate.hpp"
#include "math/vec3.hpp"


namespace jam::track
{
    /// @brief Point sampled from the track
    struct Point
    {
    public:
        /// @brief Position of the point in 3D space
        Vec3 position;

        /// @brief Normal at the control point
        Vec3 normal = Vec3(0.f, 1.f, 0.f);

        /// @brief Width at the control point
        real width = 1.f;

        /// @brief Default constructor
        inline Point() = default;

        /// @brief Default constructor
        inline Point(
            const Vec3 & position_, 
            const Vec3 & normal_, 
            real         width_
        ):
            position(position_),
            normal(normal_),
            width(width_)
        {}

        /// @brief Default destructor
        inline ~Point() = default;

    public:
        /// @brief Linera interpolation between two points
        /// @param pt0 First point
        /// @param pt1 Second point
        /// @param t   Interpolation weight
        /// @return The interpolation of the two points
        Point lerp(const Point & pt0, const Point & pt1, real t);
    };
}
