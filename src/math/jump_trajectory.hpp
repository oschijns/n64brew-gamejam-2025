#pragma once

/**
 * @file math/jump_trajectory.hpp
 * @brief Functions for computing jump trajectory parameters
 * 
 * Parameters are the following:
 * 
 * | Parameter | Description                                            |
 * |-----------|--------------------------------------------------------|
 * | height    | Height at the peak of the jump.                        |
 * | time      | Time to reach the peak of the jump.                    |
 * | impulse   | Initial vertical velocity.                             |
 * | gravity   | Gravity force applied to the body throughout the jump. |
 * 
 */

#include <cstdint>
#include "math/base.hpp"

namespace jam::jump
{
    /// @brief shorthand for power of two
    constexpr inline real pow2(real x) { return x * x; }


    // MARK: Height

    /// @param time Time to reach the peak of the jump
    /// @param impulse Initial vertical velocity 
    /// @return Height of the peak of the jump 
    constexpr inline real height_from_time_and_impulse(real time, real impulse)
    {
        return 0.5f * impulse * time;
    }

    /// @param time Time to reach the peak of the jump
    /// @param gravity Gravity applied to the body 
    /// @return Height of the peak of the jump 
    constexpr inline real height_from_time_and_gravity(real time, real gravity)
    {
        return -0.5f * gravity * pow2(time);
    }

    /// @param impulse Initial vertical velocity 
    /// @param gravity Gravity applied to the body (cannot be null)
    /// @return Height of the peak of the jump 
    constexpr inline real height_from_impulse_and_gravity(real impulse, real gravity)
    {
        return -0.5f * pow2(impulse) / gravity;
    }


    // MARK: Time

    /// @param height Height of the peak of the jump 
    /// @param impulse Initial vertical velocity (cannot be null)
    /// @return Time to reach the peak of the jump
    constexpr inline real time_from_height_and_impulse(real height, real impulse)
    {
        return 2.0f * height / impulse;
    }

    /// @param height Height of the peak of the jump 
    /// @param gravity Gravity applied to the body (cannot be null)
    /// @return Time to reach the peak of the jump
    constexpr inline real time_from_height_and_gravity(real height, real gravity)
    {
        return sqrt(2.0f * height / gravity);
    }

    /// @param impulse Initial vertical velocity 
    /// @param gravity Gravity applied to the body (cannot be null)
    /// @return Time to reach the peak of the jump
    constexpr inline real time_from_impulse_and_gravity(real impulse, real gravity)
    {
        return -impulse / gravity;
    }


    // MARK: Impulse

    /// @param height Height of the peak of the jump 
    /// @param time Time to reach the peak of the jump (cannot be null)
    /// @return Initial vertical velocity
    constexpr inline real impulse_from_height_and_time(real height, real time)
    {
        return 2.0f * height / time;
    }

    /// @param height Height of the peak of the jump 
    /// @param gravity Gravity applied to the body 
    /// @return Initial vertical velocity
    constexpr inline real impulse_from_height_and_gravity(real height, real gravity)
    {
        return sqrtf(2.0f * height * gravity);
    }

    /// @param time Time to reach the peak of the jump
    /// @param gravity Gravity applied to the body 
    /// @return Initial vertical velocity
    constexpr inline real impulse_from_time_and_gravity(real time, real gravity)
    {
        return -gravity * time;
    }


    // MARK: Gravity

    /// @param height Height of the peak of the jump 
    /// @param time Time to reach the peak of the jump (cannot be null)
    /// @return Gravity applied to the body
    constexpr inline real gravity_from_height_and_time(real height, real time)
    {
        return -2.0f * height / pow2(time);
    }

    /// @param height Height of the peak of the jump (cannot be null)
    /// @param impulse Initial vertical velocity 
    /// @return Gravity applied to the body
    constexpr inline real gravity_from_height_and_impulse(real height, real impulse)
    {
        return -0.5f * pow2(impulse) / height;
    }

    /// @param time Time to reach the peak of the jump (cannot be null)
    /// @param impulse Initial vertical velocity 
    /// @return Gravity applied to the body
    constexpr inline real gravity_from_time_and_impulse(real time, real impulse)
    {
        return -impulse / time;
    }


    // MARK: Range

    /// @param speed Horizontal speed of the body
    /// @param range Range the body can reach in a single jump
    /// @param ratio Offset ratio between ascending and descending phases
    /// @return Time to reach the peak of the jump
    constexpr inline real time_from_speed_and_range(real speed, real range, real ratio = 0.5f)
    {
        return 0.5f * ratio * range / speed;
    }
}