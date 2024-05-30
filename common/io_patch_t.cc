#include "io_patch_t.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "Enclave_t.h"

int printf(const char *fmt, ...) {
  char buf[8192] = {'\0'};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, 8192, fmt, ap);
  va_end(ap);
  ocall_printf(buf);
  return (int)strnlen(buf, 8192 - 1) + 1;
}
