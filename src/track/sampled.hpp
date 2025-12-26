#pragma once


#include <rspq.h>
#include "math/base.hpp"
#include "math/vec3.hpp"
#include "math/aabb.hpp"
#include "collection.hpp"


namespace jam::track
{
    class Model;

    /// @brief Point sampled from the track
    struct SampledPoint
    {
    public:
        /// @brief Position of the point in 3D space
        Vec3 position;

        /// @brief Normal at the control point
        Vec3 normal = Vec3(0.f, 1.f, 0.f);

        /// @brief Binormal at the control point
        Vec3 binormal = Vec3(1.f, 0.f, 0.f);

        /// @brief Width of the road at that point
        real width = 1.f;

        /// @brief Default constructor
        inline SampledPoint() = default;

        /// @brief Default constructor
        inline SampledPoint(
            const Vec3 & position_, 
            const Vec3 & normal_  , 
            const Vec3 & binormal_,
            real width_
        ):
            position(position_),
            normal  (normal_  ),
            binormal(binormal_),
            width   (width_   )
        {}

    public:
        /// @brief Linera interpolation between two points
        /// @param pt0 First point
        /// @param pt1 Second point
        /// @param t   Interpolation weight
        /// @return The interpolation of the two points
        static SampledPoint lerp(const SampledPoint & pt0, const SampledPoint & pt1, real t);

        /// @brief Write the sampled point data into the Tiny3D vertex pair data
        /// @param[in]  scale Scale for the mesh
        /// @param[in]  index Index of the sampled point in the sampled section
        /// @param[out] vert_pair a pair of vertexes
        void load_vertexes_pair(real scale, uint index, T3DVertPacked & vert_pair) const;
    };


    class SampledSection
    {
        friend class Model;

    protected:
        /// @brief List of points sampled from the initial track
        List<SampledPoint> points;

        /// @brief AABB enclosing the section 
        AABB aabb = AABB::invalid();

        /// @brief RSPQ commands to render the sub-track
        rspq_block_t * cmd_block = nullptr;

    public:

        /// @brief Destructor for the sub track
        ~SampledSection();

        /// @brief Initialize an array of indexes to build a triangle strip
        static void init_rspq_indexes();

        /// @brief Generate RSPQ command block for this sub-track
        void build_render_commands(real scale);

        /// @brief Given a position in space, compute an interpolated point 
        ///        obtain the normal and width of the track at that point.
        /// @param[in]  position The position in 3D space
        /// @param[in]  index    The index of the first vertex of the segment to check
        /// @param[out] point    The point object to populate with the closest point on the track
        /// @return New index to use for the next lookup, or which previous or next sampled sub-track to look
        /// @retval      -1  If we should look in previous sub-track (car goes in reverse)
        /// @retval     N-1  If we should look in next sub-track
        /// @retval [0, N-2] Index to use for next lookup.
        /// @details Instead of looking for the closest segment, we assume that as the 
        ///          car follows the track, it will encounter the segments in sequence.
        int closest_point(const Vec3 & position, int index, SampledPoint & point) const;
    };


    class SamplesStorage
    {
    protected:
        /// @brief Define a sequence of sampled sub-track
        /// @details They are stored as pointer so that it is quick to reorder them
        List<SampledSection *> containers;

    public:

        // TODO: write section swap method

    };
}
