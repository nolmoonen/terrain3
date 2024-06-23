#ifndef NMUTIL_DEFS_H
#define NMUTIL_DEFS_H

#include <cstdint>

typedef int nm_ret;

#define NM_SUCCESS 0
#define NM_FAIL 1

namespace nm
{
    typedef float f32;
    typedef double f64;

    typedef uint8_t u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef int8_t i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;
}

#endif // NMUTIL_DEFS_H
