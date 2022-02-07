#version 330 core

uniform mat4 mvp;

layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_col;

out vec3 val_col;

void main()
{
    gl_Position = mvp * vec4(in_pos, 1.0);
    val_col = in_col;
}