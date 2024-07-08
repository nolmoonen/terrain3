#ifndef TERRAIN3_STB_WRAPPER_H
#define TERRAIN3_STB_WRAPPER_H

int write_png(char const* filename, int x, int y, int comp, const void* data);

unsigned char* load_img(char const* filename, int* x, int* y, int* comp, int req_comp);

void free_img(unsigned char* data);

#endif // TERRAIN3_STB_WRAPPER_H
