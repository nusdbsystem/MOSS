#ifndef SECURE_SERVERLESS_COMMON_T_SEAL_H_
#define SECURE_SERVERLESS_COMMON_T_SEAL_H_

#include "sgx_trts.h"
#include "sgx_tseal.h"
#include "stdio.h"
#include "string.h"

constexpr char aad_mac_dummy[BUFSIZ] =
    "aad mac text";  // gy0106 to be replaced.

inline uint32_t get_sealed_data_size(uint32_t aad_mac_text_size,
                                     uint32_t data_to_seal_size) {
  return sgx_calc_sealed_data_size(aad_mac_text_size, data_to_seal_size);
}

inline uint32_t get_unsealed_data_size(const uint8_t* sealed_data) {
  return sgx_get_encrypt_txt_len((const sgx_sealed_data_t*)sealed_data);
}

sgx_status_t seal_data(const char* aad_mac_text, uint32_t aad_mac_text_size,
                       const char* data_to_seal, uint32_t data_to_seal_size,
                       uint8_t* output, uint32_t output_buf_size);

sgx_status_t unseal_data(const uint8_t* sealed_data, uint32_t data_size,
                         const char* aad_mac_text, uint32_t aad_mac_text_size,
                         char* output, uint32_t output_buf_size);

#endif  // SECURE_SERVERLESS_COMMON_T_SEAL_H_
