// removed the old intel code that uses openssl. use wolfssl produced by ratls.
// The codes is needed by worker enclave.
#include "base64.h"

#include "wolfssl/wolfcrypt/coding.h"

std::string base64_encode(const uint8_t* src, size_t src_size) {
  // calculate the needed buffer size
  uint32_t buf_size = (src_size + 3 - 1) / 3 * 4 + 1;
  uint8_t buf[buf_size];
  return (Base64_Encode_NoNl(src, src_size, buf, &buf_size) != 0)
             ? ""
             : std::string((char*)buf, buf_size);
}

std::string base64_decode(const uint8_t* src, size_t src_size) {
  uint32_t buf_size = (src_size * 3) >> 2;
  uint8_t buf[buf_size];
  return (Base64_Decode(src, src_size, buf, &buf_size) != 0)
             ? ""
             : std::string((char*)buf, buf_size);
}
