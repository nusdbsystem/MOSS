#ifndef SECURE_SERVERLESS_MODELS_T_MODEL_H_
#define SECURE_SERVERLESS_MODELS_T_MODEL_H_

#include <cstddef>
#include <memory>

#include "sgx_error.h"

class ModelContext {
 public:
  virtual ~ModelContext() {}
};

std::unique_ptr<ModelContext> load_model(const char* model_id,
                                         size_t model_id_len,
                                         const char* dec_key, void* store,
                                         sgx_status_t* status);

/**
 * @brief abstraction of the runtime used by a TCS for inferencing
 *
 */
class ModelRT {
 public:
  virtual ~ModelRT() {}
};

/**
 * @brief initialization of model runtime from the context.
 *  caller owns the runtime resource.
 *
 * @param ctx model context to initialize a model runtime
 * @return ModelRT* initialized model runtime. return nullptr upon failure
 */
ModelRT* model_rt_init(const ModelContext& ctx, sgx_status_t* status);

/**
 * @brief free runtime resource
 *
 * @param rt model runtime
 */
void free_model_rt(ModelRT* rt);

/**
 * @brief execute inference with decrypted input and model
 *
 * @param input_src : decrypted input binary
 * @param input_src_size
 * @param model_context: model context
 * @param model_rt: model runtime initialized by model_rt_init
 * @param output : buffer caller allocated to store output
 * @param output_buf_capacity : size of the output buffer
 * @param output_size : actual output size set by the function on return
 * @return sgx_status_t : SGX_SUCCESS for success
 */
sgx_status_t execute_inference(const char* input_src, size_t input_src_size,
                               const ModelContext& model_context,
                               ModelRT* model_rt, char* output,
                               size_t output_buf_capacity, size_t* output_size);

#endif  // SECURE_SERVERLESS_MODELS_T_MODEL_H_
