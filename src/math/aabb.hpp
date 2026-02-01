#pragma once

#include "math/base.hpp"
#include "math/vec3.hpp"

namespace jam
{
    struct AABB
    {
    public:
        // MARK: Members

        /// @brief The two end points of the AABB
        Vec3 begin, end;

        /// @brief Create a zero-initialized vector
        inline AABB(const Vec3 & begin_, const Vec3 & end_):
            begin(begin_), end(end_)
        {}

        /// @brief Copy constructor
        inline AABB(const AABB & copy): begin(copy.begin), end(copy.end) {}

        /// @brief Copy operator
        inline AABB operator=(const AABB & copy)
        {
            begin = copy.begin;
            end   = copy.end;
            return *this;
        }

        /// @brief Default destructor
        ~AABB() = default;

        /// Build an invalid AABB
        static inline AABB invalid()
        {
            return AABB(Vec3::inf_pos(), Vec3::inf_neg());
        }

        // MARK: Methods

        /// @brief Get the size of the AABB along each of its axes
        inline Vec3 size() const { return end - begin; }

        /// @brief Check if the AABB contains a point
        bool contains(const Vec3 & point) const;

        /// @brief Check if the two AABB overlaps
        static bool overlap(const AABB & a, const AABB & b);

        /// @brief Expand the AABB to include a new point
        AABB & expand_with(const Vec3 & point);

        /// @brief Expand the AABB to include another AABB
        AABB & expand_with(const AABB & aabb);


    };
}