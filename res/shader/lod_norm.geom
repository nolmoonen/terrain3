#version 330 core
layout(std140) uniform;

layout (triangles) in;
layout (line_strip, max_vertices = 2) out;

uniform mat4 uni_view_proj;

uniform float DEF_CLIPMAP_SCALE;

in float val_height[];
in vec2 val_lod[];
in float val_fog[];
in vec3 val_norm[];
in vec3 val_pos[];
in uint val_level[];

out vec3 val_color;

void main()
{
    val_color = vec3(val_norm[0].x, 0.f, val_norm[0].z);
    gl_Position = gl_in[0].gl_Position;
    EmitVertex();

    vec4 vert = vec4(
    val_pos[0] + val_norm[0] * DEF_CLIPMAP_SCALE * float(1u << val_level[0]),
    1.f);

    gl_Position = uni_view_proj * vert;
    EmitVertex();

    EndPrimitive();
}