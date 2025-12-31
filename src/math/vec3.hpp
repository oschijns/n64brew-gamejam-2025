#pragma once


#include <cstdint>
#include <t3d/t3d.h>
#include "math/base.hpp"


namespace jam
{
    union Vec3
    {
    public:
        // MARK: Members

        // Access as individual components or as an array
        struct
        {
            real x;
            real y;
            real z;
        };
        real coords[3];

        /// @brief Create a zero-initialized vector
        constexpr inline Vec3(): x(0.f), y(0.f), z(0.f) {}

        /// @brief Create a vector with specified components
        constexpr inline Vec3(real x_, real y_, real z_): x(x_), y(y_), z(z_) {}

        /// @brief Copy constructor
        constexpr inline Vec3(const Vec3 & copy): x(copy.x), y(copy.y), z(copy.z) {}

        /// @brief Copy operator
        constexpr inline void operator=(const Vec3 & copy)
        {
            x = copy.x;
            y = copy.y;
            z = copy.z;
        }

        /// @brief Default destructor
        ~Vec3() = default;

        /// @brief Return a vector with all components set to negative infinity
        static constexpr inline const Vec3 inf_neg()
        {
            return Vec3(INF_NEG, INF_NEG, INF_NEG);
        }

        /// @brief Return a vector with all components set to negative infinity
        static constexpr inline const Vec3 inf_pos()
        {
            return Vec3(INF_POS, INF_POS, INF_POS);
        }


        // MARK: Operators

        /// @brief Negate the vector
        constexpr inline Vec3 operator-() const
        {
            return Vec3(-x, -y, -z);
        }

        /// @brief Addition operator
        constexpr inline Vec3 operator+(const Vec3 & other) const
        {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        /// @brief Subtraction operator
        constexpr inline Vec3 operator-(const Vec3 & other) const
        {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        /// @brief Scalar multiplication operator
        constexpr inline Vec3 operator*(real scalar) const
        {
            return Vec3(x * scalar, y * scalar, z * scalar);
        }

        /// @brief Scalar division operator
        constexpr inline Vec3 operator/(real scalar) const
        {
            const real s = 1.0f / scalar;
            return Vec3(x * s, y * s, z * s);
        }

        /// @brief Addition assignment operator
        constexpr inline void operator+=(const Vec3 & other)
        {
            x += other.x;
            y += other.y;
            z += other.z;
        }

        /// @brief Subtraction assignment operator
        constexpr inline void operator-=(const Vec3 & other)
        {
            x -= other.x;
            y -= other.y;
            z -= other.z;
        }

        /// @brief Scalar multiplication assignment operator
        constexpr inline void operator*=(real scalar)
        {
            x *= scalar;
            y *= scalar;
            z *= scalar;
        }

        /// @brief Scalar division assignment operator
        constexpr inline void operator/=(real scalar)
        {
            const real s = 1.0f / scalar;
            x *= s;
            y *= s;
            z *= s;
        }

        /// @brief Cross product operator
        /// @note Resulting vector is orthogonal to both input vectors
        constexpr Vec3 cross(const Vec3 & other) const
        {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }

        /// @brief Dot product operator
        constexpr inline real dot(const Vec3 & other) const
        {
            return x * other.x + y * other.y + z * other.z;
        }

        /// @brief Check if the vector is zero
        constexpr inline bool is_zero() const { return x == 0.f and y == 0.f and z == 0.f; }

        /// @brief Access element by index
        constexpr inline real operator[](uint index) const { return coords[index]; }

        /// @brief Access element by index
        inline real & operator[](uint index) { return coords[index]; }


        // MARK: Methods

        /// @brief Squared magnitude of the vector
        constexpr inline real mag_sqr() const { return x * x + y * y + z * z; }

        /// @brief Magnitude of the vector
        constexpr inline real mag() const { return sqrt(mag_sqr()); }

        /// @brief Normalize the vector
        constexpr void normalize()
        {
            const real magsqr = mag_sqr();
            if (magsqr > 0.f)
            {
                const real s = 1.0f / sqrt(magsqr);
                x *= s;
                y *= s;
                z *= s;
            }
        }

        /// @brief Normalize the vector
        constexpr inline Vec3 normal()
        {
            // Copy then normalize
            Vec3 n = *this;
            n.normalize();
            return n;
        }

        /// @brief Return the projection of this vector onto another vector
        /// @note Resulting vector will be colinear to 'other'
        constexpr Vec3 projected_onto(const Vec3 & other)
        {
            return other * (dot(other) / other.mag_sqr());
        }

        /// @brief Return the rejection of this vector from another vector
        /// @note Resulting vector will be orthogonal to 'other'
        constexpr inline Vec3 rejected_from(const Vec3 & other)
        {
            return *this - projected_onto(other);
        }

        /// @brief Reflect this vector around a normal vector
        constexpr Vec3 reflected(const Vec3 & normal) const
        {
            return *this - normal * (2.0f * this->dot(normal));
        }


        // MARK: Comparison

        /// Check if the two vectors are approximately equal
        static bool approx_equal(const Vec3 & a, const Vec3 & b, real epsilon = EPSILON)
        {
            return jam::approx_equal(a.x, b.x, epsilon) and
                   jam::approx_equal(a.y, b.y, epsilon) and
                   jam::approx_equal(a.z, b.z, epsilon);
        }

        /// @brief Equality operator
        inline bool operator==(const Vec3 & other) const
        {
            return approx_equal(*this, other);
        }

        /// @brief Inequality operator
        inline bool operator!=(const Vec3 & other) const
        {
            return not approx_equal(*this, other);
        }

        /// @brief Compute squared distance between two vectors
        static constexpr inline real distance_squared(const Vec3 & a, const Vec3 & b)
        {
            return (a - b).mag_sqr();
        }

        /// @brief Compute squared distance between two vectors
        static constexpr inline real distance(const Vec3 & a, const Vec3 & b)
        {
            return (a - b).mag();
        }

        /// @brief Compute the angle between two vectors in radians
        static constexpr real angle(const Vec3 & a, const Vec3 & b)
        {
            return atan2(a.cross(b).mag(), a.dot(b));
        }

        /// @brief Check if the angle between two vectors is acute
        static constexpr bool acute(const Vec3 & a, const Vec3 & b)
        {
            return a.dot(b) > 0.f;
        }

        /// @brief Check if the angle between two vectors is orthogonal
        static constexpr bool orthogonal(const Vec3 & a, const Vec3 & b)
        {
            return abs(a.dot(b)) <= EPSILON;
        }

        /// @brief Check if the angle between two vectors is obtuse
        static constexpr bool obtuse(const Vec3 & a, const Vec3 & b)
        {
            return a.dot(b) < 0.f;
        }


        // MARK: Interpolations

        /// @brief Linear interpolation between two vectors
        static constexpr Vec3 lerp(const Vec3 & a, const Vec3 & b, real t)
        {
            const real i = 1.f - t;
            return Vec3(
                a.x * i + b.x * t,
                a.y * i + b.y * t,
                a.z * i + b.z * t
            );
        }

        /// @brief Normalized spherical linear interpolation between two vectors
        /// @note Both input vectors must be normalized
        static constexpr Vec3 normal_slerp(const Vec3 & a, const Vec3 & b, real t)
        {
            // Precompute factors
            const real
                angle = Vec3::angle(a, b),
                s     = 1.f / sin(angle),
                s0    = sin(angle * (1.f - t)),
                s1    = sin(angle * t);

            // Return the interpolated vector
            return Vec3(
                (a.x * s0 + b.x * s1) * s,
                (a.y * s0 + b.y * s1) * s,
                (a.z * s0 + b.z * s1) * s
            );
        }


        // MARK: Conversion

        /// @brief Convert the 3D vector into one compatible with Tiny3D
        inline T3DVec3 & to_t3d()
        {
            static_assert(sizeof(Vec3) == sizeof(T3DVec3),
                "Vector types cannot be trivially casted");

            // Cast our vector 3D type into Tiny3D vector type
            return *reinterpret_cast<T3DVec3*>(this);
        }

        /// @brief Convert the 3D vector into one compatible with Tiny3D
        inline explicit operator T3DVec3()
        {
            return to_t3d();
        }

        /// @brief Convert the Vector into a RSPQ vector
        /// @param[in] scale Factor to multiply the vector with
        /// @param[out] out  The RSPQ vector to write to
        constexpr inline void to_rspq(real scale, int16_t (&out) [3]) const
        {
            out[0] = (int16_t) (x * scale);
            out[1] = (int16_t) (y * scale);
            out[2] = (int16_t) (z * scale);
        }

        /// @brief Compose a RSPQ normal
        /// @return a 16-bits word containing a normal encoded for the RSPQ
        constexpr uint16_t to_rspq_normal() const
        {
            // normals are stored as 5,6,5
            // assuming that one bit is reserved for the sign of each component
            // we have S_XZ = 2^4 - 1 = 15 and S_Y = 2^5 - 1 = 31
            // TODO is it x,y,z or z,y,x ?
            constexpr real S_XZ = 15.f   , S_Y = 31.f    ;
            constexpr uint M_XZ = 0b11111, M_Y = 0b111111;

            // convert the normal into a integer
            const uint 
                ix = (int)(x * S_XZ),
                iy = (int)(y * S_Y ),
                iz = (int)(z * S_XZ);

            // combine the three components into a single 16-bits word
            const uint aggreg = 
                ((ix & M_XZ) << 11) |
                ((iy & M_Y ) <<  5) |
                ((iz & M_XZ));

            return (uint16_t) aggreg;
        }
    };
}