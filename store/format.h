#ifndef SECURE_SERVERLESS_STORE_U_FORMAT_H_
#define SECURE_SERVERLESS_STORE_U_FORMAT_H_

#include <string>

// #define LOCAL_FILE_PATH_PREFIX "tmp/"

constexpr char sealed_file_ext[] = ".ssitxt";

inline std::string CreateSealedFileName(const std::string& key) {
  return key + sealed_file_ext;
}

#endif  // SECURE_SERVERLESS_STORE_U_FORMAT_H_
