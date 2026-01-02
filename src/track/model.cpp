#include <libdragon.h>
#include "math/interpolate.hpp"
#include "track/model.hpp"


/// @brief Specify the file version expected
constexpr uint FILE_VERSION = 1;


using namespace jam;
using namespace jam::track;


// MARK: Model

void Model::sample_section(uint index, List<Point> & samples) const
{
    // Figure out where to read control points from
    const uint   idx = index * 3;
    const Vec3 * ptr = control_points.raw();

    // iterate over the path at fixed weight intervals
    const real factor = 1.f / (real) (samples.len() - 1);
    for (uint i = 0; i < samples.len(); ++i)
    {
        // precompute powers
        const real 
            t  = (real) i * factor,
            v  = 1.f - t,
            t2 = t   * t,
            t3 = t2  * t,
            v2 = v   * v,
            v3 = v2  * v;

        // Fetch control points
        const Vec3 
            & p0 = ptr[idx    ],
            & p1 = ptr[idx + 1],
            & p2 = ptr[idx + 2],
            & p3 = ptr[idx + 3];

        // Fetch section data
        const SectionData
            & s0 = sections_data[index    ],
            & s1 = sections_data[index + 1];

        // Compute the interpolated point
        // We reimplement the bezier_cubic here to avoid extra copies
        Vec3 pos;
        for (uint j = 0; j < 3; ++j)
        {
            pos.coords[j] =
                p0.coords[j] * (      v3    ) +
                p1.coords[j] * (3.f * v2 * t) +
                p2.coords[j] * (3.f * t2 * v) +
                p3.coords[j] * (      t3    );
        }
        Vec3 normal = Vec3::lerp(s0.normal, s1.normal, t);
        real width  = jam ::lerp(s0.width , s1.width , t);

        samples[i] = Point(pos, normal, width);
    }
}

/// @brief Header layout for the Track file
struct FileHeader
{
    /// @brief Which version of the file are we reading
    uint8_t version;

    /// @brief Configuration as a bitmask
    uint8_t config;

    /// @brief Number of sections to read
    uint16_t count;
};


LoadError Model::load_from_file(const char * filepath, Model & track)
{
    /*
        Data layout:

        | Size            | Element                | Description |
        |----------------:|------------------------|-------------|
        |               1 | Version                |             |
        |               1 | Configuration          | Bitmask     |
        |               2 | Number of sections (N) | 16-bits unsigned integer |
        | N *  3 * 3 * 4  | Control points         | Position of the control points using floating point values |
        | N * (3 * 4 + 4) | Section Data           | Normal and width of the track at the beginning of this section |
    */

    // Size of the components to read
    constexpr int
        SIZE_HEADER  = sizeof(FileHeader),
        SIZE_POINT   = sizeof(Vec3),
        SIZE_SECTION = sizeof(SectionData);

    // Open a file to load
    int size = 0;
    FILE * file = asset_fopen(filepath, &size);
    if (file == NULL) return LoadError::COULD_NOT_OPEN;

    // We need at the very least to read the header of the file
    if (size < SIZE_HEADER) return LoadError::EMPTY_FILE;

    // Read the header
    FileHeader header;
    fread(&header, SIZE_HEADER, 1, file);

    // check the data in the header
    if (header.version != FILE_VERSION) {
      debugf("Read version %d\n", header.version);
      return LoadError::WRONG_VERSION;
    }
    if (header.count   == 0           ) return LoadError::NO_DATA;

    // number of elements to read
    const int
        count_points   = header.count * 3,
        count_sections = header.count;

    // Check that we will be able to read the data
    auto expected_size = SIZE_HEADER + (count_points * SIZE_POINT) + (count_sections * SIZE_SECTION);
    if (size < expected_size) {
      debugf("Incorrect size: actual %d, expected: %d (%d points)\n", size, expected_size, header.count);
      return LoadError::WRONG_SECTION_COUNT;
    }

    // Prepare the lists
    track.set_section_count(header.count);

    // Read the control points (trivial mapping)
    fread((void *) track.control_points.raw_mut(), SIZE_POINT, count_points, file);
    track.control_points[count_points] = track.control_points[0];

    // Read the section data (trivial mapping)
    fread((void *) track.sections_data.raw_mut(), SIZE_SECTION, count_sections, file);
    track.sections_data[count_sections] = track.sections_data[0];

    return LoadError::OK;
}
