#ifndef SECURE_SERVERLESS_WORKER_U_WORKER_H_
#define SECURE_SERVERLESS_WORKER_U_WORKER_H_

#include <string>

#include "message/client_worker.h"
#include "sgx_urts.h"
#include "store/store.h"

class Worker {
 public:
  Worker(const std::string& enclave_file_name,
         const std::string& remote_store_path,
         const std::string& local_store_path)
      : initialize_(false),
        closed_(false),
        enclave_file_name_(std::move(enclave_file_name)),
        eid_(0),
        model_store_(OpenFsStore(std::move(remote_store_path),
                                 std::move(local_store_path))) {}

  Worker(const std::string& enclave_file_name,
         std::unique_ptr<Store> model_store)
      : initialize_(false),
        enclave_file_name_(std::move(enclave_file_name)),
        eid_(0),
        model_store_(model_store.release()) {}

  ~Worker() { Close(); }

  // delete copy and move constructors and assigment operators
  Worker(const Worker&) = delete;
  Worker& operator=(const Worker&) = delete;
  Worker(Worker&&) = delete;
  Worker& operator=(Worker&&) = delete;

  bool Initialize();

  bool Handle(uint64_t handle_id, const std::string& sample_request,
              std::string* output);

  /**
   * @brief execute the inference request in the worker managed enclave.
   *
   * @param request : user request
   * @return int : 0 for success; -1 for failure
   */
  int Execute(const WorkerRequest& request, std::string* output);

  void Close();

 private:
  bool initialize_;
  bool closed_;
  const std::string enclave_file_name_;
  sgx_enclave_id_t eid_;
  Store* model_store_;
};

#endif  // SECURE_SERVERLESS_WORKER_U_WORKER_H_
