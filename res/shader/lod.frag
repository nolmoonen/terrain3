#version 330 core
layout(std140) uniform;

uniform uint DEF_CLIPMAP_SIZE;
uniform uint DEF_CLIPMAP_LEVEL_SIZE;
uniform uint DEF_CLIPMAP_LEVEL_COUNT;
uniform float DEF_CLIPMAP_SCALE;
uniform float DEF_TEXTURE_SCALE;
uniform float DEF_TERRAIN_AMP;
uniform float DEF_TERRAIN_SCA;
uniform float DEF_TERRAIN_WATER_LVL;

uniform sampler2D grass_diff;
uniform sampler2D grass_norm;

uniform sampler2D cliff_diff;
uniform sampler2D cliff_norm;

in float val_height;
in vec2 val_lod;
in float val_fog;
in vec3 val_norm;
in vec3 val_pos;
in uint val_level;

out vec4 out_color;

void main()
{
    // simple triplanar mapping
    // https://iquilezles.org/www/articles/biplanar/biplanar.htm
    // -------------------------------------------------------------------------

    // scaling factors of the individual textures
    float grass_tex_scale = .01f;
    float cliff_tex_scale = .025f;

    vec3 x = texture(cliff_diff, cliff_tex_scale * val_pos.yz).xyz;
    vec3 y = texture(grass_diff, grass_tex_scale * val_pos.zx).xyz;
    vec3 z = texture(cliff_diff, cliff_tex_scale * val_pos.xy).xyz;

    // controls sharpness of blending in transition areas
    float k = 10.f;
    // blend factors
    vec3 w = pow(abs(val_norm), vec3(k));
    // blend
    vec3 tex_diff = (x * w.x + y * w.y + z * w.z) / (w.x + w.y + w.z);
    // todo also do tex norm and do normal mapping

    // lighting
    // -------------------------------------------------------------------------
    vec3 l = normalize(vec3(0.f, 1.f, 1.f)); // direction towards the light
    float diff = clamp(dot(l, val_norm), 0.f, 1.f);

    float ambi = .8f;

    float factor = .5f * diff + .5f * ambi;
    vec3 color = tex_diff * factor;

    color = mix(color, vec3(.5f, .6f, .7f), val_fog);

    out_color = vec4(color, 1.f);
}