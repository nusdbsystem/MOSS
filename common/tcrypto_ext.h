#ifndef SECURE_SERVERLESS_COMMON_TCRYPTOEXT_H_
#define SECURE_SERVERLESS_COMMON_TCRYPTOEXT_H_

// helper functions to use tcrypto.
#include "sgx_tcrypto.h"

#define AES_GCM_IV_SIZE 12
#define AES_GCM_TAG_SIZE 16
#define AES_GCM_AAD_SIZE 4

inline size_t get_aes_decrypted_size(size_t size) {
  return size - AES_GCM_IV_SIZE - AES_GCM_TAG_SIZE - AES_GCM_AAD_SIZE;
}

inline size_t get_aes_encrypted_size(size_t size) {
  return size + AES_GCM_IV_SIZE + AES_GCM_TAG_SIZE + AES_GCM_AAD_SIZE;
}

/**
 * @brief
 *
 * @param src
 * @param size
 * @param decryption_key
 * @param output caller is responsible to free the resources
 * @return sgx_status_t
 */
sgx_status_t decrypt_content_with_key_aes(const uint8_t* src, size_t size,
                                          const uint8_t* decryption_key,
                                          uint8_t** output);

/**
 * @brief
 *
 * @param src
 * @param size
 * @param encryption_key
 * @param output caller is responsible to free the resources.
 * @return sgx_status_t
 */
sgx_status_t encrypt_content_with_key_aes(const uint8_t* src, size_t size,
                                          const uint8_t* encryption_key,
                                          uint8_t** output);

#endif  // SECURE_SERVERLESS_COMMON_TCRYPTOEXT_H_
