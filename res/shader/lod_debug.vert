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

flat out uint val_level;
flat out uint val_id;

void main()
{
    // get world coordinate position of this vertex, see lod.vert for details
    uint level = instance[gl_InstanceID].level;
    float scale = DEF_CLIPMAP_SCALE * float(1 << level);
    vec2 local_offset = in_vertex * scale;
    vec2 mesh_pos2 =
    (instance[gl_InstanceID].offset + uni_lvl_off[level]) * DEF_CLIPMAP_SCALE;
    vec2 pos2 = mesh_pos2 + local_offset;

    // simple pattern
    float height = (sin(pos2.x) + cos(pos2.y));

    // pattern to detect degenerate generation
    //float height = .4f * (pos2.x * pos2.x + pos2.y * pos2.y);

    // zig-zag pattern to test x-axis degenerates
    //float height = float((uint(abs(pos2.y)) + 1u) % 2u);

    // zig-zag pattern to test z-axis degenerates
    //float height = float((uint(abs(pos2.x)) + 1u) % 2u);

    vec4 vert = vec4(pos2.x, height, pos2.y, 1.0);

    gl_Position = uni_view_proj * vert;

    val_level = level;
    val_id = instance[gl_InstanceID].id;
}