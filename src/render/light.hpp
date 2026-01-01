#pragma once

#include <t3d/t3d.h>
#include "math/vec3.hpp"

namespace jam::render
{
    /// @brief Maximum number of lights we can have in a scene
    constexpr uint MAX_LIGHTS = 4;


    // MARK: Light

    /// @brief Define a light in the scene
    class Light
    {
    public:
        /// @brief Direction of the light
        Vec3 direction = Vec3(0.f, 1.f, 0.f);

        /// @brief Color of the light
        color_t color = RGBA32(0xff, 0xff, 0xff, 0xff);

    public:
        /// @brief Default constructor
        constexpr inline Light() = default;

        /// @brief Construct a light for the scene
        /// @param dir Direction of the light
        /// @param color_ Color of the light
        constexpr inline Light(const Vec3 & dir, const color_t & color_):
            direction(dir),
            color(color_)
        {}

        /// @brief Default destructor
        inline ~Light() = default;

        /// @brief Copy constructor
        constexpr inline Light(const Light & other):
            direction(other.direction),
            color(other.color)
        {}

        /// @brief Copy operator
        constexpr inline void operator=(const Light & other)
        {
            direction = other.direction;
            color     = other.color;
        }

    public:
        /// @brief Render the light
        /// @param index Index of the light as it is set in the Environment
        void render(uint index);

    };


    // MARK: Environment

    /// @brief Define background colors and lighting
    class Environment
    {
    protected:
        /// @brief Lights defined in the scene
        Light lights [MAX_LIGHTS];

        /// @brief Number of lights currently in use
        uint light_count = 0;

    public:
        /// @brief Clear color for the background
        color_t clear   = RGBA32(0xff, 0xff, 0xff, 0xff);

        /// @brief Ambient color
        color_t ambient = RGBA32(0xff, 0xff, 0xff, 0xff);

    public:
        /// @brief Create a container for storing lights
        inline Environment() = default;

        /// @brief Default destructor
        inline ~Environment() = default;

        /// @brief Copy constructor
        Environment(const Environment & other) = delete;

        /// @brief Copy operator
        void operator=(const Environment & other) = delete;

    public:
        /// @brief Get the number of lights defined
        inline uint get_light_count() const { return light_count; }

        /// @brief Get the light at the specified index
        /// @param index Index of the light
        inline Light & get_light(uint index) { return lights[index]; }

        /// @brief Add a new light in the stack
        /// @param light The light to add to the stack
        void add_light(const Light & light);

        /// @brief Remove the light at the specified index
        /// @param index Index of the light to remove
        /// @note This will place the last light of the stack in its place
        void remove_light(uint index);

        /// @brief 
        void render();
    };


}