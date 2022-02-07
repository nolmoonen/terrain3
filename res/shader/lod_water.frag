#version 330 core
layout(std140) uniform;

// defines
uniform uint DEF_CLIPMAP_SIZE;
uniform uint DEF_CLIPMAP_LEVEL_SIZE;
uniform uint DEF_CLIPMAP_LEVEL_COUNT;
uniform float DEF_CLIPMAP_SCALE;
uniform float DEF_TEXTURE_SCALE;
uniform float DEF_TERRAIN_AMP;
uniform float DEF_TERRAIN_SCA;
uniform float DEF_TERRAIN_WATER_LVL;

in float val_height;
in float val_fog;

out vec4 out_color;

void main()
{
    vec3 water_color = vec3(0.f, .3f, .6f);
    float dist = val_height / DEF_TERRAIN_AMP / DEF_TERRAIN_WATER_LVL;
    // factor is at least .2f
    float factor = .2f + smoothstep(.2f, 1.f, dist);

    vec3 color = water_color * factor;
    color = mix(color, vec3(.5f, .6f, .7f), val_fog);

    out_color = vec4(color, 1.f);
}