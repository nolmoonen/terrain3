#version 330 core
layout(std140) uniform;

uniform highp sampler2DArray uni_heightmap;

uniform mat4 uni_view_proj;
uniform vec3 uni_camera_pos;
// GL doesn't allow unsized array when accessed from non-constant.
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
    per_instance_data instance[256];
};

#define LOCATION_VERTEX 0

layout(location = LOCATION_VERTEX) in uvec2 in_vertex;

out float val_height;
out float val_fog;

void main()
{
    // obtain the terrain height to determine the water color. see lod.vert
    // for details
    uint level = instance[gl_InstanceID].level;
    float flevel = float(level);
    float scale = DEF_CLIPMAP_SCALE * float(1 << level);
    vec2 local_offset = in_vertex * scale;
    vec2 mesh_pos2 =
    (instance[gl_InstanceID].offset + uni_lvl_off[level]) * DEF_CLIPMAP_SCALE;
    vec2 pos2 = mesh_pos2 + local_offset;
    ivec2 grid_pos =
    uni_lvl_off[level] + instance[gl_InstanceID].offset;
    vec2 off = fract((grid_pos / float(1 << level)) * DEF_TEXTURE_SCALE);
    vec2 texcoord = off + (in_vertex + .5f) * DEF_TEXTURE_SCALE;
    uvec2 modif =
    uvec2(uni_lvl_off[level] - uni_lvl_off[level + 1u] +
    instance[gl_InstanceID].offset);
    uvec2 v0 = (modif + ((in_vertex + uvec2(0, 0)) << level)) >> (level + 1u);
    uvec2 v1 = (modif + ((in_vertex + uvec2(0, 1)) << level)) >> (level + 1u);
    uvec2 v2 = (modif + ((in_vertex + uvec2(1, 0)) << level)) >> (level + 1u);
    uvec2 v3 = (modif + ((in_vertex + uvec2(1, 1)) << level)) >> (level + 1u);
    vec2 off_next =
    fract((uni_lvl_off[level + 1u] / float(1 << (level + 1u))) * DEF_TEXTURE_SCALE);
    vec2 texcoord_v0 = off_next + (vec2(v0) + .5f) * DEF_TEXTURE_SCALE;
    vec2 texcoord_v1 = off_next + (vec2(v1) + .5f) * DEF_TEXTURE_SCALE;
    vec2 texcoord_v2 = off_next + (vec2(v2) + .5f) * DEF_TEXTURE_SCALE;
    vec2 texcoord_v3 = off_next + (vec2(v3) + .5f) * DEF_TEXTURE_SCALE;
    vec3 tex_data_low = vec3(0.f);
    tex_data_low += texture(uni_heightmap, vec3(texcoord_v0, flevel + 1.f)).rgb;
    tex_data_low += texture(uni_heightmap, vec3(texcoord_v1, flevel + 1.f)).rgb;
    tex_data_low += texture(uni_heightmap, vec3(texcoord_v2, flevel + 1.f)).rgb;
    tex_data_low += texture(uni_heightmap, vec3(texcoord_v3, flevel + 1.f)).rgb;
    tex_data_low *= .25f;
    vec3 tex_data_high = texture(uni_heightmap, vec3(texcoord, flevel)).rgb;
    vec2 dist = abs(pos2 - uni_camera_pos.xz) * uni_inv_lvl_size[level];
    vec2 a = clamp((dist - .325f) * 8.f, 0.f, 1.f);
    float lod_factor = max(a.x, a.y);
    float terrain_height = mix(tex_data_high.x, tex_data_low.x, lod_factor);

    vec3 pos = vec3(pos2.x, DEF_TERRAIN_WATER_LVL * DEF_TERRAIN_AMP, pos2.y);
    vec4 vert = vec4(pos, 1.0);

    gl_Position = uni_view_proj * vert;
    val_height = terrain_height;

    vec3 dist_camera = uni_camera_pos - vert.xyz;
    val_fog = clamp(dot(dist_camera, dist_camera) / 25000000.f, 0.f, 1.f);
}