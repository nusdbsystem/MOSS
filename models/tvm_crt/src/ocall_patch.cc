#include <sys/time.h>

#include <cstdio>
#include <cstdlib>

#include "Enclave_u.h"

// print in microseconds time since epoch
void ocall_print_time() {
  struct timeval t;
  gettimeofday(&t, 0);
  printf("timing since start: %llu\n",
         (unsigned long long)t.tv_sec * 1000 * 1000 +
             (unsigned long long)t.tv_usec);
}

void ocall_tvm_print_string(const char* s) {
  printf("TVMCRT DEBUG PRINT: %s\n", s);
}

void ocall_tvm_crt_exit(int error_code) {
  fprintf(stderr, "TVMPlatformAbort: %d\n", error_code);
  exit(-1);
}
