#version 330 core

in vec3 val_col;

out vec4 out_col;

void main()
{
    out_col = vec4(val_col, 1.f);
}