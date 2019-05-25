#include "logger.h"

static int verbosityLevelSet = 0;

void setVerbosityLevel(int level)
{
   verbosityLevelSet = level;
}

void logger(int verbosityLevel, const char * format, ...)
{
   if (verbosityLevel <= verbosityLevelSet) {
      va_list args;
      va_start(args, format);
      vprintf(format, args);
      va_end(args);
      fflush(stdout);
   }
}
