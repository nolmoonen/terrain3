#version 330 core
layout(std140) uniform;

in vec3 val_color;

out vec4 out_color;

void main()
{
    out_color = vec4(val_color, 1.f);
}