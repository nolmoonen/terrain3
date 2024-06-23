#ifndef NMUTIL_IO_H
#define NMUTIL_IO_H

#include <cstdio>
#include "defs.h"
#include "log.h"

namespace nm {
    int read_file(char **buffer, size_t *size, const char *file_name);
}

inline nm_ret nm::read_file(
        char **buffer, size_t *size, const char *file_name)
{
    FILE *file;

    errno_t err = fopen_s(&file, file_name, "rb");
    if (!file || err) {
        nm::log(nm::LOG_ERROR, "failed to open file \"%s\"\n", file_name);

        return NM_FAIL;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    rewind(file);

    *buffer = new char[*size + 1]; // +1 for null terminator
    if (!(*buffer)) {
        nm::log(nm::LOG_ERROR, "failed to allocate %d bytes\n", *size);
        fclose(file);

        return NM_FAIL;
    }

    size_t read_size = fread(*buffer, sizeof(char), *size, file);
    fclose(file);

    if (read_size != *size) {
        nm::log(nm::LOG_ERROR, "failed to read all bytes in file\n", *size);

        return NM_FAIL;
    }

    (*buffer)[*size] = '\0';

    return NM_SUCCESS;
}

#endif // NMUTIL_IO_H
