#pragma once


#include <cstdint>
#include "math/base.hpp"
#include "math/vec3.hpp"
#include "math/quat.hpp"

namespace jam
{
    struct Isometry
    {
    public:
        // MARK: Members

        /// @brief Translation in 3D space
        Vec3 translation;

        /// @brief Rotation in 3D space
        Quat rotation;

        /// @brief Create a zero-initialized vector
        inline Isometry() = default;

        /// @brief Create a vector with specified components
        inline Isometry(const Vec3 & t, const Quat & r):
            translation(t), rotation(r)
        {}

        /// @brief Copy constructor
        inline Isometry(const Isometry & copy):
            translation(copy.translation), rotation(copy.rotation)
        {}

        /// @brief Copy operator
        inline Isometry operator=(const Isometry & copy)
        {
            translation = copy.translation;
            rotation    = copy.rotation;
            return *this;
        }

        /// @brief Default destructor
        ~Isometry() = default;
    };
}