// core/logsys.hpp
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    LOG_NONE = 0,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
} loglevel_t;

#define LOG_DEFAULT(...) \
    Log_Message(LOG_NONE, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_WARNING(...) \
    Log_Message(LOG_WARNING, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_ERROR(...) \
    Log_Message(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#define LOG_FATAL(...) \
    Log_Message(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void Log_Init(const char* out);
void Log_Shutdown(void);
void Log_Message(loglevel_t level, const char* file, int line, const char* fmt, ...)
    __attribute__((format(printf, 4, 5)));

#ifdef __cplusplus
}
#endif