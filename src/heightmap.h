#ifndef TERRAIN3_HEIGHTMAP_H
#define TERRAIN3_HEIGHTMAP_H

#include "nmutil/vector.h"
#include "terrain_defs.h"
#include "nmutil/gl.h"

/// This file and its implementation encapsulate the heightmap, which is the
/// texture that represents the heights and the gradients of the terrain.

/// As we move in the world, we overwrite the parts of the texture as needed.
/// Each grid position at which (and only at which) a vertex can be located,
/// maps to a single texel.
/// Texel coordinates are grid coordinates divided by 2^level.
/// Grid coordinates are world coordinates scaled with CLIPMAP_SCALE.

/// Maintains the state of a single level's texture.
struct level_info {
    /// (-x,-y)-most world texel coordinate of the currently loaded texture,
    /// if cleared is true.
    int32_t x;
    int32_t y;
    bool cleared;
};

/// Maintains information about a texture region that should be recomputed as
/// the part of the world that it represents has changed. As well as information
/// on where to update it in the texture.
struct update_info {
    /// Local texel coordinate of (-x/-y)-most point of region.
    nm::ivec2 tex;
    // todo should this not be unsigned?
    /// Size in texels of the region.
    nm::ivec2 size;
    /// World texel coordinate of (-x/-y)-most point of region.
    nm::ivec2 start;
    /// Level of the texture.
    uint32_t level;
    float padding0;
};

struct heightmap {
    /// Texture containing the heightmap and normal.
    nm::tex texture;
    GLuint uniform_buffer;
    size_t uniform_buffer_size;

    nm::tex noise_tex;

    /// One level info for each level.
    level_info level_infos[CLIPMAP_LEVEL_COUNT];
    /// Each level can at most generate 4 for x-dimension and 4 for y-dimension.
#define MAX_UPDATE_COUNT (CLIPMAP_LEVEL_COUNT * 8u)

    /// Has to be a power of two. This is used to generate the terrain.
#define NOISE_SIZE 256
    uint8_t *noise;
};

nm_ret init(heightmap *hm);

void cleanup(heightmap *hm);

void update(heightmap *hm, nm::ivec2 level_offsets[CLIPMAP_LEVEL_COUNT]);

/// Returns a height in [0,1] of a world-space position.
nm::fvec3 get_height(heightmap *hm, nm::fvec2 pos);

/// Encapsulation for applying the heightmap texture.
void use_texture(heightmap *hm);

void unuse_texture(heightmap *hm);

#endif //TERRAIN3_HEIGHTMAP_H