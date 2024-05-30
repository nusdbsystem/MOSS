#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

#include "Enclave_t.h"
#include "common/tcrypto_ext.h"
#include "config.h"
#include "models/include/model.h"
#include "src/bundle.h"

#define OUTPUT_LEN (OUTPUT_BUF_SZ / sizeof(float))
#define TVM_CRT_MEMORY_BUFFER_SZ MODELRT_BUF_SZ

namespace {
class ModelContextImpl : public ModelContext {
 public:
  ModelContextImpl(char* graph, size_t graph_len, char* params_data,
                   size_t params_len)
      : graph_(graph),
        graph_len_(graph_len),
        params_data_(params_data),
        params_len_(params_len) {}
  ~ModelContextImpl() {
    ocall_tvm_print_string("model context deleted");
    if (graph_) free(graph_);
    if (params_data_) free(params_data_);
  }

  // delete copy and move
  ModelContextImpl(const ModelContextImpl&) = delete;
  ModelContextImpl(ModelContextImpl&&) = delete;
  ModelContextImpl& operator=(const ModelContextImpl&) = delete;
  ModelContextImpl& operator=(ModelContextImpl&&) = delete;

  const char* graph() const { return graph_; }
  size_t graph_len() const { return graph_len_; }
  const char* params_data() const { return params_data_; }
  size_t params_len() const { return params_len_; }

 private:
  char* graph_;
  size_t graph_len_;
  char* params_data_;
  size_t params_len_;
};

class ModelRTImpl : public ModelRT {
 public:
  ModelRTImpl(uint8_t* buf, void* handle)
      : tvm_memory_buffer_(buf), handle_(handle) {}

  ~ModelRTImpl() {
    ocall_tvm_print_string("model rt deleted");
    if (handle_) tvm_runtime_destroy(handle_);
    handle_ = nullptr;
    if (tvm_memory_buffer_) free(tvm_memory_buffer_);
    tvm_memory_buffer_ = nullptr;
  }

  void* handle() { return handle_; }

 private:
  uint8_t* tvm_memory_buffer_;
  void* handle_;
};
}  // anonymous namespace

ModelRT* model_rt_init(const ModelContext& ctx, sgx_status_t* status) {
  uint8_t* buf = (uint8_t*)malloc(TVM_CRT_MEMORY_BUFFER_SZ);
  if (!buf) {
    *status = SGX_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  const ModelContextImpl& mc = static_cast<const ModelContextImpl&>(ctx);
  void* handle =
      tvm_runtime_create(mc.graph(), mc.params_data(), mc.params_len(), buf,
                         TVM_CRT_MEMORY_BUFFER_SZ);
  ocall_print_time();
  return new ModelRTImpl{buf, handle};
}

void free_model_rt(ModelRT* rt) {
  delete (rt);
  rt = nullptr;
}

namespace {
const char graph_file_suffix[] = ".graph_json";
const char params_file_suffix[] = ".params_data";
constexpr int suffix_max_len =
    (sizeof(graph_file_suffix) > sizeof(params_file_suffix))
        ? sizeof(graph_file_suffix)
        : sizeof(params_file_suffix);

size_t load_and_decode(const char* id, size_t len, const char* dec_key,
                       void* store, char** out, sgx_status_t* status) {
  char* loaded = nullptr;
  size_t loaded_len = 0;
  *status = ocall_load_content(id, len, &loaded, &loaded_len, store);
  if (*status != SGX_SUCCESS) return 0;
  char* cipher_text = (char*)malloc(loaded_len + 1);
  if (cipher_text == NULL) {
    *status = SGX_ERROR_OUT_OF_MEMORY;
    ocall_free_loaded(id, len, store);
    return 0;
  }
  memcpy(cipher_text, loaded, loaded_len);
  cipher_text[loaded_len] = '\0';
  ocall_free_loaded(id, len, store);

  // decrypt
  uint8_t* content = nullptr;
  *status =
      decrypt_content_with_key_aes((const uint8_t*)cipher_text, loaded_len,
                                   (const uint8_t*)dec_key, &content);
  free(cipher_text);
  cipher_text = nullptr;
  if (*status != SGX_SUCCESS) return 0;

  // prepare return
  *out = reinterpret_cast<char*>(content);
  // for nenc
  // *out = reinterpret_cast<char*>(cipher_text);
  return loaded_len;
};
}  // anonymous namespace

std::unique_ptr<ModelContext> load_model(const char* model_id,
                                         size_t model_id_len,
                                         const char* dec_key, void* store,
                                         sgx_status_t* status) {
  assert(status);
  assert(dec_key);
  assert(model_id);
  assert(model_id_len > 0);

  char fetch_id[model_id_len + suffix_max_len];
  memcpy(fetch_id, model_id, model_id_len);
  memcpy(fetch_id + model_id_len, graph_file_suffix, sizeof(graph_file_suffix));
  char* graph = nullptr;
  size_t graph_len =
      load_and_decode(fetch_id, model_id_len + sizeof(graph_file_suffix),
                      dec_key, store, &graph, status);
  if (!graph) return nullptr;

  memcpy(fetch_id + model_id_len, params_file_suffix,
         sizeof(params_file_suffix));
  char* params = nullptr;
  size_t params_len =
      load_and_decode(fetch_id, model_id_len + sizeof(params_file_suffix),
                      dec_key, store, &params, status);
  if (!params) {
    free(graph);
    return nullptr;
  }

  return std::unique_ptr<ModelContext>(
      new ModelContextImpl(graph, graph_len, params, params_len));
}

sgx_status_t execute_inference(const char* input_src, size_t input_src_size,
                               const ModelContext& /*ctx*/, ModelRT* model_rt,
                               char* output_buf, size_t output_buf_capacity,
                               size_t* output_size) {
  void* handle = static_cast<ModelRTImpl*>(model_rt)->handle();

  DLTensor input;
  input.data = (char*)input_src;
  DLDevice dev = {kDLCPU, 0};
  input.device = dev;
  input.ndim = 4;
  DLDataType dtype = {kDLFloat, 32, 1};
  input.dtype = dtype;
  int64_t shape[4] = {1, 3, 224, 224};
  input.shape = shape;
  input.strides = NULL;
  input.byte_offset = 0;

  tvm_runtime_set_input(handle, "data", &input);

  ocall_print_time();

  tvm_runtime_run(handle);

  ocall_print_time();

  float output_storage[OUTPUT_LEN];
  DLTensor output;
  output.data = output_storage;
  DLDevice out_dev = {kDLCPU, 0};
  output.device = out_dev;
  output.ndim = 2;
  DLDataType out_dtype = {kDLFloat, 32, 1};
  output.dtype = out_dtype;
  int64_t out_shape[2] = {1, OUTPUT_LEN};
  output.shape = out_shape;
  output.strides = NULL;
  output.byte_offset = 0;

  tvm_runtime_get_output(handle, 0, &output);

  ocall_print_time();

  int output_sz = OUTPUT_LEN * sizeof(float);
  if (output_sz > output_buf_capacity) {
    return SGX_ERROR_UNEXPECTED;
  }

  memcpy(output_buf, output_storage, output_sz);
  *output_size = output_sz;

  ocall_print_time();

  return SGX_SUCCESS;
}
