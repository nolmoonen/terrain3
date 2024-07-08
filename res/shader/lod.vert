#version 330 core
layout(std140) uniform;

uniform highp sampler2DArray uni_heightmap;

uniform mat4 uni_view_proj;
uniform vec3 uni_camera_pos;
// GL doesnt allow unsized array when accessed from non-constant
uniform float uni_inv_lvl_size[10];
uniform ivec2 uni_lvl_off[10];

// defines
uniform uint DEF_CLIPMAP_SIZE;
uniform uint DEF_CLIPMAP_LEVEL_SIZE;
uniform uint DEF_CLIPMAP_LEVEL_COUNT;
uniform float DEF_CLIPMAP_SCALE;
uniform float DEF_TEXTURE_SCALE;
uniform float DEF_TERRAIN_AMP;
uniform float DEF_TERRAIN_SCA;
uniform float DEF_TERRAIN_WATER_LVL;

// note: copied definition from mesh.h
struct per_instance_data {
    ivec2 offset;
    uint level;
    uint id;
};

uniform uni_instance_data {
// set to support at least the maximum number of instances created
    per_instance_data instance[256];
};

#define LOCATION_VERTEX 0
layout(location = LOCATION_VERTEX) in uvec2 in_vertex;

out float val_river;
out float val_height;
out vec2 val_lod;
out float val_fog;
out vec3 val_norm;
out vec3 val_pos;
out uint val_level;

void main()
{
    val_level = instance[gl_InstanceID].level;
    float flevel = float(val_level);

    // coverts a 'local' grid coordinate to a world coordinate
    float scale = DEF_CLIPMAP_SCALE * float(1 << val_level);
    // world coordinate relative to mesh
    vec2 local_offset = in_vertex * scale;

    // position of this mesh in world coordinates
    vec2 mesh_pos2 =
    (instance[gl_InstanceID].offset + uni_lvl_off[val_level]) * DEF_CLIPMAP_SCALE;

    // position of this vertex in world coordinates
    vec2 pos2 = mesh_pos2 + local_offset;

    // position in grid
    ivec2 grid_pos =
    uni_lvl_off[val_level] + instance[gl_InstanceID].offset;
    // scale down to 2^level, take fract to increase precision
    // which is valid since we use GL_REPEAT
    vec2 off = fract((grid_pos / float(1 << val_level)) * DEF_TEXTURE_SCALE);

    // .5f offset to sample mid-texel
    vec2 texcoord = off + (in_vertex + .5f) * DEF_TEXTURE_SCALE;

    // vector from the next level's offset to this block in grid space
    // note: this is always positive
    uvec2 modif =
    uvec2(uni_lvl_off[val_level] - uni_lvl_off[val_level + 1u] +
    instance[gl_InstanceID].offset);

    // the four sample points, aligned with the grid of the next level
    // w.r.t. the current level's mesh
    uvec2 v0 = (modif + ((in_vertex + uvec2(0, 0)) << val_level)) >> (val_level + 1u);
    uvec2 v1 = (modif + ((in_vertex + uvec2(0, 1)) << val_level)) >> (val_level + 1u);
    uvec2 v2 = (modif + ((in_vertex + uvec2(1, 0)) << val_level)) >> (val_level + 1u);
    uvec2 v3 = (modif + ((in_vertex + uvec2(1, 1)) << val_level)) >> (val_level + 1u);

    // texture_offset for the (0,0) block on the next level
    vec2 off_next =
    fract((uni_lvl_off[val_level + 1u] / float(1 << (val_level + 1u))) * DEF_TEXTURE_SCALE);

    // todo only do interpolation when lod_factor > 0.f and not highest level

    // .5f offset to sample mid-texel
    vec2 texcoord_v0 = off_next + (vec2(v0) + .5f) * DEF_TEXTURE_SCALE;
    vec2 texcoord_v1 = off_next + (vec2(v1) + .5f) * DEF_TEXTURE_SCALE;
    vec2 texcoord_v2 = off_next + (vec2(v2) + .5f) * DEF_TEXTURE_SCALE;
    vec2 texcoord_v3 = off_next + (vec2(v3) + .5f) * DEF_TEXTURE_SCALE;

    // bilinear filtering:
    // obtain low resolution height and normal by interpolating between
    // four grid-aligned points on a lower resolution (higher level) texture
    vec4 tex_data_low = vec4(0.f);
    tex_data_low += texture(uni_heightmap, vec3(texcoord_v0, flevel + 1.f));
    tex_data_low += texture(uni_heightmap, vec3(texcoord_v1, flevel + 1.f));
    tex_data_low += texture(uni_heightmap, vec3(texcoord_v2, flevel + 1.f));
    tex_data_low += texture(uni_heightmap, vec3(texcoord_v3, flevel + 1.f));
    tex_data_low *= .25f;

    // high-resolution: x is height, yz is gradient
    vec4 tex_data_high = texture(uni_heightmap, vec3(texcoord, flevel));

    // find blending factors for heightmap. the detail level must not have
    // any discontinuities or it shows as 'artifacts'.
    vec2 dist = abs(pos2 - uni_camera_pos.xz) * uni_inv_lvl_size[val_level];
    vec2 a = clamp((dist - .325f) * 8.f, 0.f, 1.f);
    float lod_factor = max(a.x, a.y);
    float terrain_height = mix(tex_data_high.x, tex_data_low.x, lod_factor);

    vec3 norm_high = normalize(vec3(-tex_data_high.y, 1.f, -tex_data_high.z));
    vec3 norm_low = normalize(vec3(-tex_data_low.y, 1.f, -tex_data_low.z));

    val_norm = normalize(mix(norm_high, norm_low, lod_factor));

    val_river = mix(tex_data_high.w, tex_data_low.w, lod_factor);

    val_height = terrain_height;
    val_pos = vec3(pos2.x, val_height, pos2.y);
    vec4 vert = vec4(val_pos, 1.0);

    gl_Position = uni_view_proj * vert;
    val_lod = vec2(val_level, lod_factor);

    vec3 dist_camera = uni_camera_pos - vert.xyz;
    // per-vertex fog
    val_fog = clamp(dot(dist_camera, dist_camera) / 25000000.f, 0.f, 1.f);
}