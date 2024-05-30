#ifndef SECURE_SERVERLESS_COMMON_BASE64_H_
#define SECURE_SERVERLESS_COMMON_BASE64_H_

#include <string>

std::string base64_encode(const uint8_t* src, size_t src_size);
std::string base64_decode(const uint8_t* src, size_t src_size);

#endif  // SECURE_SERVERLESS_COMMON_BASE64_H_
