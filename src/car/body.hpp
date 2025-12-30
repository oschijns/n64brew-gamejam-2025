#pragma once


#include "math/base.hpp"
#include "math/isometry.hpp"


namespace jam::car
{
    /// @brief Body of the car
    class Body
    {
    public:
        /// @brief Isometry of the car in 3D space
        Isometry isometry;

        /// @brief Velocity of the car
        Vec3 velocity;

        /// @brief Section of the track this body is in
        uint16_t section = 0;

        /// @brief Segment of the sampled section this body is on
        uint16_t segment = 0;

        /// @brief Default constructor
        inline Body() = default;

        /// @brief Default constructor
        inline Body(const Isometry & iso):
            isometry(iso)
        {}
    };
}
