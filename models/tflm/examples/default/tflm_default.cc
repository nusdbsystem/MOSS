/**
 * implemented with reference to tensorflow/tflite-micro person detection
 *  example and Jumpst3r/tensorflow-lite-sgx sample
 */

#include <cstdint>
#include <memory>

#include "Enclave_t.h"
#include "common/tcrypto_ext.h"
#include "config.h"
#include "models/include/model.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"

#ifndef NDEBUG
#include "common/io_patch_t.h"
#endif  // NDEBUG

#define TensorArenaSize MODELRT_BUF_SZ

namespace {
class ModelContextImpl : public ModelContext {
 public:
  ModelContextImpl(char* content, size_t len)
      : content_(content), content_len_(len) {}
  ~ModelContextImpl() {
    if (content_) free(content_);
  }

  // delete copy and move
  ModelContextImpl(const ModelContextImpl&) = delete;
  ModelContextImpl(ModelContextImpl&&) = delete;
  ModelContextImpl& operator=(const ModelContextImpl&) = delete;
  ModelContextImpl& operator=(ModelContextImpl&&) = delete;

  const char* content() const { return content_; }
  size_t len() const { return content_len_; }

 private:
  char* content_;
  size_t content_len_;
};

class ModelRTImpl : public ModelRT {
 public:
  ModelRTImpl()
      : model_(nullptr),
        micro_error_reporter_(),
        interpreter_(nullptr),
        tensor_arena_(nullptr) {}
  ~ModelRTImpl() {
    if (tensor_arena_) free(tensor_arena_);
    tensor_arena_ = nullptr;
  }

  sgx_status_t maybe_init(const char* model_buf) {
    if (model_ != model_buf) {
      tflite::InitializeTarget();
      model_ = model_buf;
      const tflite::Model* model =
          tflite::GetModel(reinterpret_cast<const int8_t*>(model_buf));
      if (model->version() != TFLITE_SCHEMA_VERSION) {
#ifndef NDEBUG
        printf(
            "Model provided is schema version %d not equal to \
          supported version %d\n",
            model->version(), TFLITE_SCHEMA_VERSION);
#endif  // NDEBUG
        return SGX_ERROR_UNEXPECTED;
      }
      int kTensorArenaSize = TensorArenaSize;
      if (!tensor_arena_) {
        tensor_arena_ = (uint8_t*)malloc(kTensorArenaSize);
        if (!tensor_arena_) return SGX_ERROR_OUT_OF_MEMORY;
      }
      memset(tensor_arena_, 0, kTensorArenaSize);
      interpreter_.reset(new tflite::MicroInterpreter(
          model, resolver_, tensor_arena_, kTensorArenaSize,
          &micro_error_reporter_));
      auto status = interpreter_->AllocateTensors();
      if (status != kTfLiteOk) {
#ifndef NDEBUG
        printf("AllocateTensors() failed\n");
#endif  // NDEBUG
        return SGX_ERROR_UNEXPECTED;
      }
    }
  }

  tflite::MicroInterpreter* interpreter() { return interpreter_.get(); }

 private:
  const char* model_;  // does not own.
  tflite::MicroErrorReporter micro_error_reporter_;
  tflite::AllOpsResolver
      resolver_;  // resolver lifetime needs to be longer than interpreter.
  std::unique_ptr<tflite::MicroInterpreter> interpreter_;  // ownership
  uint8_t* tensor_arena_;                                  // ownership
};

constexpr char model_file_suffix[] = ".model";
}  // anonymous namespace

ModelRT* model_rt_init(const ModelContext& ctx, sgx_status_t* status) {
  ModelRTImpl* ret = new ModelRTImpl();
  *status =
      ret->maybe_init(static_cast<const ModelContextImpl&>(ctx).content());
  if (*status != SGX_SUCCESS) {
    delete ret;
    return nullptr;
  }
  ocall_print_time();
  return ret;
}

void free_model_rt(ModelRT* rt) {
  delete (rt);
  rt = nullptr;
}

std::unique_ptr<ModelContext> load_model(const char* model_id,
                                         size_t model_id_len,
                                         const char* dec_key, void* store,
                                         sgx_status_t* status) {
  assert(status);
  assert(dec_key);
  assert(model_id);
  assert(model_id_len > 0);

  char* loaded{nullptr};
  size_t load_len{0};

  char fetch_id[model_id_len + sizeof(model_file_suffix)];
  memcpy(fetch_id, model_id, model_id_len);
  memcpy(fetch_id + model_id_len, model_file_suffix, sizeof(model_file_suffix));
  size_t fetch_id_len = sizeof(fetch_id);
  ocall_load_content(fetch_id, fetch_id_len, &loaded, &load_len, store);
  // copy to enclave memory
  char* cipher_text = (char*)malloc(load_len + 1);
  if (cipher_text == NULL) {
    *status = SGX_ERROR_OUT_OF_MEMORY;
    ocall_free_loaded(fetch_id, fetch_id_len, store);
    return NULL;
  }
  memcpy(cipher_text, loaded, load_len);
  cipher_text[load_len] = '\0';
  ocall_free_loaded(fetch_id, fetch_id_len, store);

  // decrypt
  uint8_t* content;
  *status = decrypt_content_with_key_aes((const uint8_t*)cipher_text, load_len,
                                         (const uint8_t*)dec_key, &content);
  free(cipher_text);
  cipher_text = nullptr;
  if (*status != SGX_SUCCESS)
    return NULL;  // note that the caller only return a buffer if success.

  // prepare return
  return std::unique_ptr<ModelContext>(
      new ModelContextImpl((char*)content, load_len));
}

sgx_status_t execute_inference(const char* input_src, size_t input_src_size,
                               const ModelContext& model_context,
                               ModelRT* model_rt, char* output,
                               size_t output_buf_capacity,
                               size_t* output_size) {
  ModelRTImpl* rt = reinterpret_cast<ModelRTImpl*>(model_rt);
  rt->maybe_init(static_cast<const ModelContextImpl&>(model_context).content());

  const int8_t* tfinput = reinterpret_cast<const int8_t*>(input_src);
  auto input = rt->interpreter()->input(0);
  memcpy(input->data.raw, tfinput, input->bytes);

  auto status = rt->interpreter()->Invoke();
  if (status != kTfLiteOk) {
#ifndef NDEBUG
    printf("Invoke() failed\n");
#endif  // NDEBUG
    return SGX_ERROR_UNEXPECTED;
  }

  // copy result into output
  auto output_sz = rt->interpreter()->output(0)->bytes;
  if (output_sz > output_buf_capacity) {
#ifndef NDEBUG
    printf("Not enough output buffer\n");
#endif  // NDEBUG
    return SGX_ERROR_UNEXPECTED;
  }

  memcpy(output, rt->interpreter()->output(0)->data.raw, output_sz);
  *output_size = output_sz;

  return SGX_SUCCESS;
}
