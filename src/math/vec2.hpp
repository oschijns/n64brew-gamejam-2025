#pragma once


#include <cstdint>
#include "math/base.hpp"


namespace jam
{
    union Vec2
    {
    public:
        // MARK: Members

        // Access as individual components or as an array
        struct
        {
            real x;
            real y;
        };
        real coords[2];

        /// @brief Create a zero-initialized vector
        inline Vec2(): x(0), y(0) {}

        /// @brief Create a vector with specified components
        inline Vec2(real x_, real y_): x(x_), y(y_) {}

        /// @brief Copy constructor
        inline Vec2(const Vec2 & copy): x(copy.x), y(copy.y) {}

        /// @brief Copy operator
        inline Vec2 operator=(const Vec2 & copy)
        {
            x = copy.x;
            y = copy.y;
            return *this;
        }

        /// @brief Default destructor
        ~Vec2() = default;

        /// @brief Return a vector with all components set to negative infinity
        static inline const Vec2 inf_neg()
        {
            return Vec2(INF_NEG, INF_NEG);
        }

        /// @brief Return a vector with all components set to negative infinity
        static inline const Vec2 inf_pos()
        {
            return Vec2(INF_POS, INF_POS);
        }


        // MARK: Operators

        /// @brief Negate the vector
        inline Vec2 operator-() const
        {
            return Vec2(-x, -y);
        }

        /// @brief Addition operator
        inline Vec2 operator+(const Vec2 & other) const
        {
            return Vec2(x + other.x, y + other.y);
        }

        /// @brief Subtraction operator
        inline Vec2 operator-(const Vec2 & other) const
        {
            return Vec2(x - other.x, y - other.y);
        }

        /// @brief Scalar multiplication operator
        inline Vec2 operator*(real scalar) const
        {
            return Vec2(x * scalar, y * scalar);
        }

        /// @brief Scalar division operator
        inline Vec2 operator/(real scalar) const
        {
            const real s = 1.0f / scalar;
            return Vec2(x * s, y * s);
        }

        /// @brief Addition assignment operator
        inline Vec2 & operator+=(const Vec2 & other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        /// @brief Subtraction assignment operator
        inline Vec2 & operator-=(const Vec2 & other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        /// @brief Scalar multiplication assignment operator
        inline Vec2 & operator*=(real scalar)
        {
            x *= scalar;
            y *= scalar;
            return *this;
        }

        /// @brief Scalar division assignment operator
        inline Vec2 & operator/=(real scalar)
        {
            const real s = 1.0f / scalar;
            x *= s;
            y *= s;
            return *this;
        }

        /// @brief Cross product operator
        inline real cross(const Vec2 & other) const
        {
            return x * other.y + y * other.x;
        }

        /// @brief Dot product operator
        inline real dot(const Vec2 & other) const
        {
            return x * other.x + y * other.y;
        }

        /// @brief Check if the vector is zero
        inline bool is_zero() const { return x == 0.f and y == 0.f; }

        /// @brief Access element by index
        inline real operator[](uint index) const { return coords[index]; }

        /// @brief Access element by index
        inline real & operator[](uint index) { return coords[index]; }


        // MARK: Methods

        /// @brief Squared magnitude of the vector
        inline real mag_sqr() const { return x * x + y * y; }

        /// @brief Magnitude of the vector
        inline real mag() const { return sqrt(mag_sqr()); }

        /// @brief Normalize the vector
        Vec2 & normalize()
        {
            const real magsqr = mag_sqr();
            if (magsqr > 0.f)
            {
                const real s = 1.0f / sqrt(magsqr);
                x *= s;
                y *= s;
            }
            return *this;
        }

        /// @brief Normalize the vector
        inline Vec2 normal()
        {
            Vec2 n = *this;
            n.normalize();
            return n;
        }

        /// @brief Return the projection of this vector onto another vector
        /// @note Resulting vector will be colinear to 'other'
        Vec2 projected_onto(const Vec2 & other)
        {
            return other * (dot(other) / other.mag_sqr());
        }

        /// @brief Return the rejection of this vector from another vector
        /// @note Resulting vector will be orthogonal to 'other'
        inline Vec2 rejected_from(const Vec2 & other)
        {
            return *this - projected_onto(other);
        }

        /// @brief Reflect this vector around a normal vector
        Vec2 reflected(const Vec2 & normal) const
        {
            return *this - normal * (2.0f * this->dot(normal));
        }


        // MARK: Comparison

        /// Check if the two vectors are approximately equal
        static bool approx_equal(const Vec2 & a, const Vec2 & b, real epsilon = EPSILON)
        {
            return jam::approx_equal(a.x, b.x, epsilon) and
                   jam::approx_equal(a.y, b.y, epsilon);
        }

        /// @brief Equality operator
        inline bool operator==(const Vec2 & other) const
        {
            return approx_equal(*this, other);
        }

        /// @brief Inequality operator
        inline bool operator!=(const Vec2 & other) const
        {
            return not approx_equal(*this, other);
        }

        /// @brief Compute squared distance between two vectors
        static inline real distance_squared(const Vec2 & a, const Vec2 & b)
        {
            return (a - b).mag_sqr();
        }

        /// @brief Compute squared distance between two vectors
        static inline real distance(const Vec2 & a, const Vec2 & b)
        {
            return (a - b).mag();
        }

        /// @brief Compute the angle between two vectors in radians
        static real angle(const Vec2 & a, const Vec2 & b)
        {
            return atan2(a.cross(b), a.dot(b));
        }

        /// @brief Check if the angle between two vectors is acute
        static bool acute(const Vec2 & a, const Vec2 & b)
        {
            return a.dot(b) > 0.f;
        }

        /// @brief Check if the angle between two vectors is orthogonal
        static bool orthogonal(const Vec2 & a, const Vec2 & b)
        {
            return abs(a.dot(b)) <= EPSILON;
        }

        /// @brief Check if the angle between two vectors is obtuse
        static bool obtuse(const Vec2 & a, const Vec2 & b)
        {
            return a.dot(b) < 0.f;
        }


        // MARK: Interpolations

        /// @brief Linear interpolation between two vectors
        static Vec2 lerp(const Vec2 & a, const Vec2 & b, real t)
        {
            const real i = 1.f - t;
            return Vec2(
                a.x * i + b.x * t,
                a.y * i + b.y * t
            );
        }

        /// @brief Normalized spherical linear interpolation between two vectors
        /// @note Both input vectors must be normalized
        static Vec2 normal_slerp(const Vec2 & a, const Vec2 & b, real t)
        {
            // Precompute factors
            const real
                angle = Vec2::angle(a, b),
                s     = 1.f / sin(angle),
                s0    = sin(angle * (1.f - t)),
                s1    = sin(angle * t);

            // Return the interpolated vector
            return Vec2(
                (a.x * s0 + b.x * s1) * s,
                (a.y * s0 + b.y * s1) * s
            );
        }


        // MARK: Conversion

        /// @brief Convert the Vector into a RSPQ vector
        /// @param scale Factor to multiply the vector with
        /// @param[out] out  The RSPQ vector to write to
        inline void to_rspq(real scale, int16_t (&out) [2]) const
        {
            out[0] = (int16_t) (x * scale);
            out[1] = (int16_t) (y * scale);
        }
    };
}