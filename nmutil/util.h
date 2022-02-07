#ifndef NMUTUL_UTIL_H
#define NMUTUL_UTIL_H

/// Round up to nearest aligned offset. Align is a power of 2.
inline size_t realign_offset(size_t offset, size_t align)
{
    // basically ((offset + align - 1) / align) * align
    return (offset + align - 1) & ~(align - 1);
}

#endif // NMUTUL_UTIL_H
