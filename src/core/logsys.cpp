#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "core/logsys.hpp"

static FILE* outfile = NULL;

void Log_Init( const char* out ){
  outfile = fopen(out, "a");
  if (!outfile)
  {
    fprintf(stderr, "Failed to open log file\n");
    exit(1);
  }
}


void Log_Shutdown( void ){
  if (outfile)
    fclose(outfile);
  outfile = NULL;
}

void Log_Message(loglevel_t level, const char* file, int line, const char* fmt, ...)
{
  static const char* levels[] =
  {
    "LOG",
    "WARNING",
    "ERROR",
    "FATAL"
  };

  fprintf(stderr, "[%s] %s:%d: ", levels[level], file, line);
  if (outfile)
  {
    fprintf(outfile, "[%s] %s:%d: ", levels[level], file, line);
  }
  
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");

  if (outfile)
  {
    va_start(args, fmt);
    vfprintf(outfile, fmt, args);
    va_end(args);
    fprintf(outfile, "\n");
    fflush(outfile);
  }
  if (level == LOG_FATAL)
    exit(1);
}
