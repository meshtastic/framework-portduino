#include "Utility.h"
#include <Print.h>

#include <cstdarg>

#define MAX_STR_LEN 256

size_t Print::printf(const char *format, ...) {
    char buf[MAX_STR_LEN]; // FIXME, this burns a lot of stack, but on
                                // Linux that is fine, TBD on MyNewt
    va_list args;
    va_start(args, format);
    int n = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    if (n <= 0) return 0;
    // vsnprintf returns the would-be length; clamp to what actually fit.
    size_t len = (n < (int)sizeof(buf)) ? (size_t)n : sizeof(buf) - 1;
    return write((const uint8_t *)buf, len);
}