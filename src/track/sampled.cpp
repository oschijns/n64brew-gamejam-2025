#include <libdragon.h>
#include <t3d/t3d.h>
#include "math/vec2.hpp"
#include "math/plane.hpp"
#include "math/interpolate.hpp"
#include "track/sampled.hpp"


/// @brief Buffer size for triangle strips to generate
constexpr uint BUFFER_SIZE = 32;


using namespace jam;
using namespace jam::track;


// MARK: Sampled Point

SampledPoint lerp(const SampledPoint & pt0, const SampledPoint & pt1, real t)
{
    return SampledPoint(
        Vec3::lerp        (pt0.position, pt1.position, t),
        Vec3::normal_slerp(pt0.normal  , pt1.normal  , t),
        Vec3::normal_slerp(pt0.binormal, pt1.binormal, t),
        jam ::lerp        (pt0.width   , pt1.width   , t)
    );
}


void SampledPoint::load_vertexes_pair(real scale, uint index, T3DVertPacked & vert_pair) const
{
    // for now the tracks will be pure white
    constexpr uint32_t WHITE = 0xFFFFFFFF;

    // Scaling for UV mapping
    constexpr real UV_SCALE = 32.f;

    // Compute the two points
    const Vec3 
        half = binormal * (width * 0.5),
        pt_a = position - half,
        pt_b = position + half;

    // write positions
    pt_a.to_rspq(scale, vert_pair.posA);
    pt_b.to_rspq(scale, vert_pair.posB);

    // write normals
    const uint16_t norm = normal.to_rspq_normal();
    vert_pair.normA = norm;
    vert_pair.normB = norm;

    // write colors
    vert_pair.rgbaA = WHITE;
    vert_pair.rgbaB = WHITE;

    // write UV mapping
    const real height = (real) index;
    const Vec2
        st_a (0.f, height),
        st_b (1.f, height);
    st_a.to_rspq(UV_SCALE, vert_pair.stA);
    st_b.to_rspq(UV_SCALE, vert_pair.stB);
}


/// @brief Sub process for the `SampledPoint::collide` method, 
///        handle one of the three plane delimiting the road.
/// @param plane    Plane limit to check for contact
/// @param radius   Radius of the object
/// @param position Position of the object
/// @param velocity Velocity of the object
/// @return Return true if there was a contact with the plane
bool inner_collide(const Plane & plane, real radius, Vec3 & position, Vec3 & velocity)
{
    const real distance = plane.distance(position);

    // If the sphere around the object is intersecting or below the plane
    // Then there is a contact.
    if (distance < radius)
    {
        // Object is under the road, push it out of it
        position += plane.normal * abs(radius - distance);
        velocity  = plane.normal.rejected_from(velocity);
        return true;
    }
    else return false;
}


bool SampledPoint::collide(real radius, Vec3 & position_, Vec3 & velocity) const
{
    // Build three planes from the point: 
    // one for the floor and two for the walls
    const Vec3 side = binormal * (width * 0.5f);
    const Plane
        plane_floor ( normal  , position),
        plane_left  ( binormal, position - side),
        plane_right (-binormal, position + side);

    // Check contact with the floor for sure
    const bool contact_floor = inner_collide(plane_floor, radius, position_, velocity);

    // Check contact with either walls
    // Here it is fine if C++ do some weird optimization, the two walls should 
    // not be so close that an object is in contact with both at the same time.
    const bool contact_wall  = 
            inner_collide(plane_left , radius, position_, velocity) ||
            inner_collide(plane_right, radius, position_, velocity);

    // Return if there was any contact
    return contact_floor || contact_wall;
}


// MARK: Sampled Section

/// @brief Array of indexes to initialize once
static int16_t RSPQ_INDEXES [BUFFER_SIZE * 2];

void SampledSection::init_rspq_indexes()
{
    for (uint i = 0; i < BUFFER_SIZE * 2; ++i)
        RSPQ_INDEXES[i] = i;
}


// positions are set as signed 16-bits integer
// we should pick a base scale factor to properly convert a f32 into a s16
void SampledSection::build_render_commands(real scale)
{
    // To draw the track, we need vertices on each edge of the road. 
    // Because we have N sampled points, and a T3DVertPacked contains two points,
    // we can simply declare a array of N T3DVertPacked.
    static T3DVertPacked vertices [BUFFER_SIZE];

    // number of points to process
    const uint count = points.len();

    // write the sampled points to the RSPQ buffer
    for (uint i = 0; i < count; ++i)
        points[i].load_vertexes_pair(scale, i, vertices[i]);

    rspq_block_begin();
    t3d_vert_load(vertices, 0, count); // TODO: do we need an offset?
    t3d_tri_draw_strip(RSPQ_INDEXES, count);

    // TODO: do we need other commands to properly setup our mesh?

    cmd_block = rspq_block_end();
}

SampledSection::~SampledSection()
{
    if (cmd_block)
    {
        rspq_wait();
        rspq_block_free(cmd_block);
    }
}


int SampledSection::closest_point(const Vec3 & position, int index, SampledPoint & point) const
{
    // as long as we are within the sub-track, look for a segment
    while (-1 < index and index < (int) points.len() - 1)
    {
        // using the index, pick two consecutive points to form as segment
        const SampledPoint 
            & pt0 = points[index    ], 
            & pt1 = points[index + 1];

        // Direction from the first point of the segment to the next one
        const Vec3 dir = pt1.position - pt0.position;

        // Get a projection scalar of the position onto the segment pt0 -> pt1
        real t = (position - pt0.position).dot(dir) / dir.mag_sqr();

        // if that projection is 
        // - lower than zero, we have moved to the previous segment
        // - higher than one, we have moved to the next segment
        if      (t < 0.0) --index;
        else if (t > 1.0) ++index;
        else
        {
            point = SampledPoint::lerp(pt0, pt1, t);
            return index;
        }
    }
    return index;
}


// MARK: Sampled Storage

SampledSection * SamplesStorage::append(SampledSection * section)
{
    // size of the container
    const uint count = containers.len() - 1;

    // pick the head of the list
    SampledSection *const head = containers[0];

    // shift the array toward the beginning
    memmove(containers.raw_mut(), containers.raw() + 1, count);

    // add the section at the last position
    containers[count] = section;

    return head;
}

SampledSection * SamplesStorage::prepend(SampledSection * section)
{
    // size of the container
    const uint count = containers.len() - 1;

    // pick the tail of the list
    SampledSection *const tail = containers[count];

    // shift the array toward the end
    memmove(containers.raw_mut() + 1, containers.raw(), count);

    // add the section at the first position
    containers[0] = section;

    return tail;
}