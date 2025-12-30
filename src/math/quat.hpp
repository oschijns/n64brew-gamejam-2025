#pragma once


#include <cstdint>
#include "math/base.hpp"
#include "math/vec3.hpp"

namespace jam
{
    union Quat
    {
    public:
        // MARK: Members

        // Access as individual components or as an array
        struct
        {
            real x;
            real y;
            real z;
            real w;
        };
        real coords[4];

        /// @brief Create a zero-initialized quaternion
        constexpr inline Quat(): x(0.f), y(0.f), z(0.f), w(1.f) {}

        /// @brief Create a quaternion with specified components
        constexpr inline Quat(real x_, real y_, real z_, real w_):
            x(x_), y(y_), z(z_), w(w_)
        {}

        /// @brief Copy constructor
        constexpr inline Quat(const Quat & copy):
            x(copy.x), y(copy.y), z(copy.z), w(copy.w)
        {}

        /// @brief Copy operator
        constexpr inline Quat operator=(const Quat & copy)
        {
            x = copy.x;
            y = copy.y;
            z = copy.z;
            w = copy.w;
            return *this;
        }

        /// @brief Default destructor
        ~Quat() = default;


        // MARK: Operators

        /// @brief Inverse the quaternion
        /// @note Must be normalized for the inversion to succeed.
        constexpr inline Quat inv() const
        {
            return Quat(-x, -y, -z, w);
        }

        /// @brief Negate the quaternion
        constexpr inline Quat operator-() const
        {
            return Quat(-x, -y, -z, -w);
        }

        /// @brief Addition operator
        constexpr inline Quat operator+(const Quat & other) const
        {
            return Quat(x + other.x, y + other.y, z + other.z, w + other.w);
        }

        /// @brief Subtraction operator
        constexpr inline Quat operator-(const Quat & other) const
        {
            return Quat(x - other.x, y - other.y, z - other.z, w - other.w);
        }

        /// @brief Scalar multiplication operator
        constexpr inline Quat operator*(real scalar) const
        {
            return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        /// @brief Scalar division operator
        constexpr inline Quat operator/(real scalar) const
        {
            const real s = 1.0f / scalar;
            return Quat(x * s, y * s, z * s, w * s);
        }

        /// @brief Quaternion product
        constexpr Quat operator*(const Quat & o) const
        {
            return Quat(
                w * o.x + x * o.w + y * o.z - z * o.y,
                w * o.y + y * o.w + z * o.x - x * o.z,
                w * o.z + z * o.w + x * o.y - y * o.x,
                w * o.w - x * o.x - y * o.y - z * o.z
            );
        }

        /// @brief Addition assignment operator
        constexpr inline void operator+=(const Quat & other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
        }

        /// @brief Subtraction assignment operator
        constexpr inline void operator-=(const Quat & other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
        }

        /// @brief Scalar multiplication assignment operator
        constexpr inline void operator*=(real scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
            w *= scalar;
        }

        /// @brief Scalar division assignment operator
        constexpr inline void operator/=(real scalar)
        {
            const real s = 1.0f / scalar;
            x *= s;
            y *= s;
            z *= s;
            w *= s;
        }

        /// @brief Quaternion product
        constexpr void operator*=(const Quat & o)
        {
            const real
                x_ = w * o.x + x * o.w + y * o.z - z * o.y,
                y_ = w * o.y + y * o.w + z * o.x - x * o.z,
                z_ = w * o.z + z * o.w + x * o.y - y * o.x,
                w_ = w * o.w - x * o.x - y * o.y - z * o.z;

            // replace components
            x = x_;
            y = y_;
            z = z_;
            w = w_;
        }

        constexpr inline Vec3 operator*(const Vec3 & vec) const
        {
            // get an axis orthogonal to provided vector
            const Vec3
                quat  = Vec3(x, y, z),
                ortho = quat.cross(vec);

		    return vec + (ortho * w + quat.cross(ortho)) * 2.f;
        }

        /// @brief Dot product operator
        constexpr inline real dot(const Quat & other) const
        {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        /// @brief Access element by index
        constexpr inline real operator[](uint index) const { return coords[index]; }

        /// @brief Access element by index
        inline real & operator[](uint index) { return coords[index]; }


        // MARK: Methods

        /// @brief Squared magnitude of the quaternion
        constexpr inline real mag_sqr() const { return x * x + y * y + z * z + w * w; }

        /// @brief Magnitude of the quaternion
        constexpr inline real mag() const { return sqrt(mag_sqr()); }

        /// @brief Normalize the quaternion
        constexpr void normalize()
        {
            const real magsqr = mag_sqr();
            if (magsqr > 0.f)
            {
                const real s = 1.0f / sqrt(magsqr);
                x *= s;
                y *= s;
                z *= s;
                w *= s;
            }
        }

        /// @brief Normalize the quaternion
        constexpr inline Quat normal()
        {
            // Copy then normalize
            Quat n = *this;
            n.normalize();
            return n;
        }

        /// @brief Return the projection of this quaternion onto another quaternion
        /// @note Resulting quaternion will be colinear to 'other'
        constexpr Quat projected_onto(const Quat & other)
        {
            return other * (dot(other) / other.mag_sqr());
        }

        /// @brief Return the rejection of this quaternion from another quaternion
        /// @note Resulting quaternion will be orthogonal to 'other'
        constexpr inline Quat rejected_from(const Quat & other)
        {
            return *this - projected_onto(other);
        }

        /// @brief Reflect this quaternion around a normal quaternion
        constexpr Quat reflected(const Quat & normal) const
        {
            return *this - normal * (2.0f * this->dot(normal));
        }


        // MARK: Comparison

        /// Check if the two quaternions are approximately equal
        static constexpr bool approx_equal(const Quat & a, const Quat & b, real epsilon = EPSILON)
        {
            return jam::approx_equal(a.x, b.x, epsilon) and
                   jam::approx_equal(a.y, b.y, epsilon) and
                   jam::approx_equal(a.z, b.z, epsilon) and
                   jam::approx_equal(a.w, b.w, epsilon);
        }

        /// @brief Equality operator
        constexpr inline bool operator==(const Quat & other) const
        {
            return approx_equal(*this, other);
        }

        /// @brief Inequality operator
        constexpr inline bool operator!=(const Quat & other) const
        {
            return not approx_equal(*this, other);
        }

        /// @brief Compute squared distance between two quaternions
        static constexpr inline real distance_squared(const Quat & a, const Quat & b)
        {
            return (a - b).mag_sqr();
        }

        /// @brief Compute squared distance between two quaternions
        static constexpr inline real distance(const Quat & a, const Quat & b)
        {
            return (a - b).mag();
        }

    };
}