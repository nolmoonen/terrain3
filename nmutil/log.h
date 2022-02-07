#ifndef NMUTIL_LOG_H
#define NMUTIL_LOG_H

#include <cstdarg>
#include <cstdio>

namespace nmutil {
    static const char *const LEVEL_NAMES[] = {
            "TRACE", "INFO ", "WARN ", "ERROR"
    };

    enum log_level_e {
        LOG_TRACE = 0,
        LOG_INFO,
        LOG_WARN,
        LOG_ERROR
    };

    static log_level_e m_level = LOG_TRACE;

    /// Sets the log level.
    static void set_log_level(log_level_e t_level);

    /// Log.
    static void log(log_level_e t_level, const char *t_format, ...);

    inline void set_log_level(log_level_e t_level)
    {
        m_level = t_level;
    }

    inline void log(log_level_e t_level, const char *t_format, ...)
    {
        if (t_level >= m_level) {
            fprintf(stdout, "%s ", LEVEL_NAMES[t_level]);
            va_list argptr;
                    va_start(argptr, t_format);
            vfprintf(stdout, t_format, argptr);
                    va_end(argptr);
        }
    }
}

#endif // NMUTIL_LOG_H
