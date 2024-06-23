#ifndef NMUTIL_LOG_H
#define NMUTIL_LOG_H

#include <cstdarg>
#include <cstdio>

// use #define NM_LOG_IMPLEMENTATION in before including in one file

namespace nm {
enum log_level_e { LOG_TRACE = 0, LOG_INFO, LOG_WARN, LOG_ERROR };

/// Log.
void log(log_level_e t_level, const char* t_format, ...);

void set_log_level(log_level_e t_level);

#ifdef NM_LOG_IMPLEMENTATION

static const char* const LEVEL_NAMES[] = {"TRACE", "INFO ", "WARN ", "ERROR"};

static log_level_e m_level = LOG_INFO;

void log(log_level_e t_level, const char* t_format, ...)
{
    if (t_level >= m_level) {
        fprintf(stdout, "%s ", LEVEL_NAMES[t_level]);
        va_list argptr;
        va_start(argptr, t_format);
        vfprintf(stdout, t_format, argptr);
        va_end(argptr);
    }
}

void set_log_level(log_level_e t_level)
{
    m_level = t_level;
}

#endif // NM_LOG_IMPLEMENTATION

} // namespace nm

#endif // NMUTIL_LOG_H