#pragma once

#include <math.h>
#include "math/base.hpp"

namespace jam
{
    /// @brief Linear interpolation between two values
    template<typename T>
    inline T lerp(const T & a, const T & b, real t)
    {
        return a * (1.f - t) + b * t;
    }

    /// @brief Quadratic Bezier curve interpolation
    template<typename T>
    inline T bezier_quad(const T (&points) [3], real t)
    {
        const real i = 1.f - t;
        return
            points[0] * (      i * i) +
            points[1] * (2.f * i * t) +
            points[2] * (      t * t);
    }

    /// @brief Derivative of the quadratic Bezier curve interpolation
    template<typename T>
    inline T bezier_quad_der(const T (&points) [3], real t)
    {
        return (
            (points[1] - points[0]) * (1.f - t) +
            (points[2] - points[1]) * t
        ) * 2.f;
    }

    /// @brief Second derivative of the quadratic Bezier curve interpolation
    template<typename T>
    inline T bezier_quad_sec(const T (&points) [3], real t)
    {
        return (points[2] - points[1] * 2.f + points[0]) * 2.f;
    }

    /// @brief Cubic interpolation between two values
    template<typename T>
    inline T bezier_cubic(const T (&points) [4], real t)
    {
        // precompute powers
        const real i = 1.f - t, t2 = t * t, i2 = i * i;
        return
            points[0] * (      i2 * i) +
            points[1] * (3.f * i2 * t) +
            points[2] * (3.f * t2 * i) +
            points[3] * (      t2 * t);
    }

    /// @brief Derivative of the cubic interpolation between two values
    template<typename T>
    inline T bezier_cubic_der(const T (&points) [4], real t)
    {
        // precompute powers
        const real i = 1.f - t;
        return (
            (points[1] - points[0]) * (      i * i) +
            (points[2] - points[1]) * (2.f * i * t) +
            (points[3] - points[2]) * (      t * t)
        ) * 3.f;
    }

    /// @brief Derivative of the cubic interpolation between two values
    template<typename T>
    inline T bezier_cubic_sec(const T (&points) [4], real t)
    {
        return (
            (points[2] - points[1] * 2.f + points[0]) * (1.f - t) +
            (points[3] - points[2] * 2.f + points[1]) * t
        ) * 6.f;
    }
}
