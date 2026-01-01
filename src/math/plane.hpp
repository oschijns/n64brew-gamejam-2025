#pragma once


#include "math/vec3.hpp"


namespace jam
{
    struct Plane
    {
    public:
        /// @brief Normal of the plane
        Vec3 normal = Vec3(0.f, 1.f, 0.f);

        /// @brief Offset from the origin
        real offset = 0;

    public:
        /// @brief Default constructor
        constexpr inline Plane() = default;


        /// @brief Create a plane from a normal and an offset
        /// @param normal_ Normal of the plane
        /// @param offset_ Offset from the origin
        constexpr inline Plane(const Vec3 & normal_, real offset_):
            normal(normal_), offset(offset_)
        {}


        /// @brief Create a plane from a normal and a point on the plane
        /// @param normal_ Normal of the plane
        /// @param point Point on the plane
        constexpr inline Plane(const Vec3 & normal_, const Vec3 & point):
            normal(normal_), offset(normal_.dot(point))
        {}


        /// @brief Default destructor
        inline ~Plane() = default;


        /// @brief Copy constructor
        inline Plane(const Plane & other):
            normal(other.normal),
            offset(other.offset)
        {}


        /// @brief Copy operator
        inline void operator=(const Plane & other)
        {
            normal = other.normal;
            offset = other.offset;
        }

    public:
        /// @brief Normalize the plane
        constexpr inline void normalize() { normal.normalize(); }

    public:
        /// @brief Signed distance between this plane and the provided point
        /// @param point Point over this plane
        /// @return Signed distance between the two
        /// @note If the distance is negative, the point is below the plane
        constexpr inline real distance(const Vec3 & point) const
        {
            return normal.dot(point) - offset;
        }

        /// @brief Project the provided point so that it lies on the plane
        /// @param point The point to project
        /// @return The point projected to the plane
        constexpr inline Vec3 project(const Vec3 & point) const
        {
            return point - normal * distance(point);
        }
    };
}