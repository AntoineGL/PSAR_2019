#pragma once

#include <stdarg.h>
#include <stdio.h>

/**
 * setVerbosityLevel - Set verbosity level for log
 *
 * @level: the level of verbosity chosen
 */
void setVerbosityLevel(int level);

/**
 * log - Print log if verbosityLevel is lower or equal than the level set
 *       with setVerbosityLevel.
 *
 * @verbosityLevel: verbosity level
 * @fmt:            what to print
 */
void logger(int verbosityLevel, const char * fmt, ...);
