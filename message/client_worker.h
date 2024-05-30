#ifndef SECURE_SERVERLESS_MESSAGE_U_CLIENTWORKER_H_
#define SECURE_SERVERLESS_MESSAGE_U_CLIENTWORKER_H_

#include <memory>
#include <string>

#include "request.h"

struct WorkerRequest : public Request {
  WorkerRequest() = default;
  ~WorkerRequest() = default;

  WorkerRequest(const WorkerRequest& other) = default;
  WorkerRequest(WorkerRequest&& other) = default;
  WorkerRequest& operator=(const WorkerRequest& other) = default;
  WorkerRequest& operator=(WorkerRequest&& other) = default;

  /**
   * @brief encode the request into a string
   */
  std::string EncodeTo() const override;

  std::string encrypted_sample_;
  std::string model_name_;
  std::string user_id_;
  std::string key_service_address_;
  uint16_t key_service_port_;
};

WorkerRequest DecodeWorkerRequest(const std::string& request);

#endif  // SECURE_SERVERLESS_MESSAGE_U_CLIENTWORKER_H_
