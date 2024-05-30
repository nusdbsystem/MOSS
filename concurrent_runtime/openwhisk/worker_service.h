#ifndef SECURE_SERVERLESS_CONCURRENTRT_U_OW_WORKERSERVICE_H_
#define SECURE_SERVERLESS_CONCURRENTRT_U_OW_WORKERSERVICE_H_

#include <cstdlib>
#include <mutex>
#include <string>

#include "service.h"
#include "worker.h"

class WorkerService : public Service {
 public:
  // the logic of worker and where it fetch and store data are configured
  //  in the worker_config.h
  WorkerService(const std::string& enclave_path,
                const std::string& filestore_path,
                const std::string& store_path)
      // : worker_(std::move(enclave_path), std::move(filestore_path),
      //   std::move(store_path)) {
      : worker_(std::move(enclave_path),
                std::unique_ptr<Store>(OpenFsStore(filestore_path, "", 0))) {
    printf(
        "Worker service config:\n enclave file name: %s\n\
        store path: %s, filestore path %s\n",
        enclave_path.c_str(), store_path.c_str(), filestore_path.c_str());
  }
  ~WorkerService() = default;

  response_code_t Init(const std::string& request,
                       std::string* response) override;

  // run will also attemp to init if it has not done so.
  //  this allows interfacing with serverless platform that only
  //  exposes one http endpoint
  response_code_t Run(const std::string& request,
                      std::string* response) override;

 private:
  std::once_flag initalized_;
  Worker worker_;
};

#endif  // SECURE_SERVERLESS_CONCURRENTRT_U_OW_WORKERSERVICE_H_
