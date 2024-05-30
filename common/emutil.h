#ifndef SECURE_SERVERLESS_COMMON_EMUTIL_H_
#define SECURE_SERVERLESS_COMMON_EMUTIL_H_

/**
 * @brief enclave memory abstraction to read it like a file
 *  and a list of helper methods. emem is a view and does not take
 *  ownership of the underlying resource.
 *
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  const char* buf;
  const char* curr;
  size_t size;
} emem;

// modeled after fgetl in sgx-dnet
// Note that both depletion of emem and malloc error return NULL.
//  caller is responsible to check the emem curr for emem depletion.
char* emem_getline(emem* m);

// similar to fread but for byte read.
size_t emem_read(void* dest, size_t size, emem* m);

bool emem_reached_end(const emem* m);

#endif  // SECURE_SERVERLESS_COMMON_EMUTIL_H_
