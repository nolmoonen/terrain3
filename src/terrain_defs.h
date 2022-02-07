#ifndef TERRAIN3_TERRAIN_DEFS_H
#define TERRAIN3_TERRAIN_DEFS_H

// mesh parameters -------------------------------------------------------------

/// Sets the size of clipmap blocks, NxN vertices per block.
/// Should be power-of-two and no bigger than 64.
/// A clipmap-level is organized roughly as 4x4 blocks with some padding.
/// A clipmap level is a (4N-1) * (4N-1) grid.
#define CLIPMAP_SIZE 64u

// todo mesh is not set up to handle sizes greater than 64
static_assert(CLIPMAP_SIZE <= 64u, "");

/// The dimension of one level of the mesh in number of vertices.
#define CLIPMAP_LEVEL_SIZE (4u * CLIPMAP_SIZE - 1u)

/// Number of LOD levels for clipmap.
#define CLIPMAP_LEVEL_COUNT 10u

// todo the shaders are not set up to handle values > 10
static_assert(CLIPMAP_LEVEL_COUNT <= 10u, "");

/// Distance between vertices.
#define CLIPMAP_SCALE .1f

/// Scale factor to convert local offsets (vertex coordinates) into
/// texture coordinates.
#define TEXTURE_SCALE (1.f / CLIPMAP_LEVEL_SIZE)

// terrain parameters ----------------------------------------------------------

/// Terrain amplitude.
#define TERRAIN_AMP 300.f

/// Terrain scale.
#define TERRAIN_SCA .0006f

/// Water level, before scaling. World height is roughly in [0,2].
#define TERRAIN_WATER_LVL .6f

#endif //TERRAIN3_TERRAIN_DEFS_H
