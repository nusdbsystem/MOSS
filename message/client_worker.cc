#include "client_worker.h"

#include <cassert>
#include <fstream>

#include "common/base64.h"
#include "common/json.h"

std::string WorkerRequest::EncodeTo() const {
  json::JSON ret;
  ret["encrypted_sample"] = encrypted_sample_;
  ret["model_type"] = model_name_;
  ret["user_id"] = user_id_;
  ret["key_service_address"] = key_service_address_;
  ret["key_service_port"] = key_service_port_;
  return ret.dump();
}

WorkerRequest DecodeWorkerRequest(const std::string& request) {
  // size_t decoded_size;
  // char* decoded_user_input = base64_decode(request.data(), &decoded_size);
  // auto input = json::JSON::Load(decoded_user_input);
  // free(decoded_user_input);
  auto input = json::JSON::Load(request);
  auto encrypted_sample_src = input["encrypted_sample"].ToString();
  WorkerRequest ret;
  ret.encrypted_sample_ = base64_decode(
      (const uint8_t*)encrypted_sample_src.data(), encrypted_sample_src.size());
  ret.model_name_ = input["model_type"].ToString();
  ret.user_id_ = input["user_id"].ToString();
  ret.key_service_address_ = input["key_service_address"].ToString();
  assert(input["key_service_port"].ToInt() < UINT16_MAX);
  ret.key_service_port_ =
      static_cast<uint16_t>(input["key_service_port"].ToInt());
  return ret;
}
