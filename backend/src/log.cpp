#include "log.h"

#include <stdio.h>

#include <cstdarg>

void log(const char *message, ...) {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
}

void error(const char *message, ...) {
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    fprintf(stderr, "\n");
    fflush(stderr);
}
