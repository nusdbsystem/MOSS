#ifndef SECURE_SERVERLESS_MODELS_CONFIG_H_
#define SECURE_SERVERLESS_MODELS_CONFIG_H_

// configs to enforce the memory buffers sizes and times for processing inside
// enclave.

/**
 * @brief fixed buffer size for storing plaintext
 *  image_224_f32: 602112
 *  image_299_f32: 1072812
 */
#define INPUT_BUF_SZ 602112
/**
 * @brief fixed buffer size for storing the output and its encryption
 * tflm sample models: 4004, 1000 imagenet + 1 background classes.
 * tvm sample models: 4000, it dictate the output size of tvm runtime output
 *  so incorrect value will cause tvm runtime to crash.
 */
#define OUTPUT_BUF_SZ 4000

/**
 * @brief config the model runtime memory consumption for
 *  storing models and intermediates
 *
 * tflm:
 *  mobilenet_v1_1.0_224 5000000
 *  resnet_v2_101_299 23600000
 *  densenet 12000000
 * tvm crt:
 *  mobilenet1.0: 30 MB
 *  resnet101_v2: 205 MB
 *  densenet121: 55 MB
 */
#define MODELRT_BUF_SZ (30 << 20)

/**
 * @brief equalize a sandbox processing time
 *  (init one fixed model and fetch key everytime)
 */
#define EQUAL_EXEC_TIME true

/**
 * @brief force the use of model inside enclave,
 *  when EQUAL_EXEC_TIME is true
 */
#define PERMITTED_MODEL                                               \
  "2c984d551a5ababe621790aeb8bdd90bf679ce895f784ab3374ef02528c85b59-" \
  "mobilenet1.0"

#endif  // SECURE_SERVERLESS_MODELS_CONFIG_H_
