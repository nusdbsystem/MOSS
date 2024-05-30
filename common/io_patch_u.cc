#include <sys/time.h>

#include <cstdio>
#include <cstdlib>

#include "Enclave_u.h"

void ocall_printf(const char *str) { printf("%s", str); }

// print in microseconds time since epoch
void ocall_print_time() {
  struct timeval t;
  gettimeofday(&t, 0);
  printf("timing since start: %llu\n",
         (unsigned long long)t.tv_sec * 1000 * 1000 +
             (unsigned long long)t.tv_usec);
}
