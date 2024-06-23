#define STB_IMAGE_IMPLEMENTATION

#include "../external/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../external/stb/stb_image_write.h"
#include "nmutil/log.h"

int write_png(char const *filename, int x, int y, int comp, const void *data)
{
    stbi_flip_vertically_on_write(1);
    return stbi_write_png(filename, x, y, comp, data, 0);
}

unsigned char *load_img(
        char const *filename, int *x, int *y, int *comp, int req_comp)
{
    unsigned char *data = stbi_load(filename, x, y, comp, req_comp);
    if (!data) {
        nm::log(nm::LOG_WARN, "could not load texture %s\n", filename);

        return nullptr;
    }

    return data;
}

void free_img(unsigned char *data)
{ stbi_image_free(data); }