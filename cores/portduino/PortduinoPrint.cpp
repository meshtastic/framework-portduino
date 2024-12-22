#include "Utility.h"
#include <Print.h>

#include <cstdarg>

#define MAX_STR_LEN 256

size_t Print::printf(const char *format, ...) {
    char buf[MAX_STR_LEN]; // FIXME, this burns a lot of stack, but on
                                // Linux that is fine, TBD on MyNewt
    va_list args;
    va_start(args, format);
    size_t n = vsnprintf(buf, sizeof(buf), format, args);
    write(buf);
    va_end(args);
    return n;
}