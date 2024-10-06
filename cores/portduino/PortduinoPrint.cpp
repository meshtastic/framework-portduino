#include "Utility.h"
#include <Print.h>

#include <cstdarg>

//#define MAX_STR_LEN 256
#define PRINTF_BUFFER_SIZE 250 // by JG

size_t Print::printf(const char *format, ...) {
  /* old:
  char buf[MAX_STR_LEN]; // FIXME, this burns a lot of stack, but on
                                // Linux that is fine, TBD on MyNewt
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf), format, args);
  write(buf);
  va_end(args);
  */
  
  // by JG
  char buf[PRINTF_BUFFER_SIZE];
  va_list args;
  va_start(args, format);
  int status = vsnprintf(buf, PRINTF_BUFFER_SIZE, format, args);
  va_end(args);
  if (status >= 0) {
    buf[PRINTF_BUFFER_SIZE - 1] = '\0';
    size_t n = print(buf);
    return n;
  } else {
    return 0;
  }
}     
