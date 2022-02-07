#version 330 core

uniform uint DEF_CLIPMAP_LEVEL_COUNT;

flat in uint val_level;
flat in uint val_id;

out vec4 out_col;

void main() {
    // debug the types
    float fact = (DEF_CLIPMAP_LEVEL_COUNT - val_level) * .1f;
    if (val_id == 0u) { // quadlet
        out_col = vec4(fact * vec3(1.f, 1.f, 0.f), 1.f);// yellow
    } else if (val_id == 1u) { // quad
        out_col = vec4(fact * vec3(1.f, 1.f, 1.f), 1.f);// white
    } else if (val_id == 2u) { // level zero fixup
        out_col = vec4(fact * vec3(0.f, 0.f, 1.f), 1.f);// blue
    } else if (val_id == 3u) { // fixup
        out_col = vec4(fact * vec3(0.f, 1.f, 0.f), 1.f);// green
    } else if (val_id == 7u) { // trim
        out_col = vec4(fact * vec3(1.f, 0.f, 1.f), 1.f);// magenta
    }

    // degenerate colors

    else if (val_id == 3u) {
        out_col = vec4(fact * vec3(1.f, .5f, .5f), 1.f);// -x: light red
    } else if (val_id == 4u) {
        out_col = vec4(fact * vec3(1.f, 0.f, 0.f), 1.f);// +x: red
    } else if (val_id == 5u) {
        out_col = vec4(fact * vec3(.5f, .5f, 1.f), 1.f);// -z: light blue
    } else if (val_id == 6u) {
        out_col = vec4(fact * vec3(0.f, 0.f, 1.f), 1.f);// +z: blue
    }
}