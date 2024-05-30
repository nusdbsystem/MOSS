#ifndef SECURE_SERVERLESS_COMMON_ENDIAN_H_
#define SECURE_SERVERLESS_COMMON_ENDIAN_H_

#include <stdint.h>

/**
 * @brief change byte endianness
 *
 */
uint32_t change_endian(uint32_t val) {
  val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
  return (val << 16) | (val >> 16);
}

#endif  // SECURE_SERVERLESS_COMMON_ENDIAN_H_
