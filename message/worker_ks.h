#ifndef SECURE_SERVERLESS_MESSAGE_WORKERKS_H_
#define SECURE_SERVERLESS_MESSAGE_WORKERKS_H_

#include <string>

#include "request.h"

class GetKeyRequest : public Request {
 public:
  GetKeyRequest(const std::string& user_id, const std::string& model_id)
      : user_id_(user_id), model_id_(model_id) {}
  ~GetKeyRequest() = default;

  std::string EncodeTo() const override;

  static GetKeyRequest DecodeFrom(const std::string& src);

  inline const std::string& user_id() const { return user_id_; }
  inline const std::string& model_id() const { return model_id_; }

 private:
  const std::string user_id_;
  const std::string model_id_;
};

struct KeyServiceWorkerReply : public Reply {
  KeyServiceWorkerReply(const std::string& error_msg)
      : success_(false), msg_(error_msg) {}
  KeyServiceWorkerReply(const std::string& user_id,
                        const std::string& input_key,
                        const std::string& model_id,
                        const std::string& model_key);
  ~KeyServiceWorkerReply() = default;
  std::string EncodeTo() const override;
  bool success_;
  std::string msg_;
};

extern int DecodeKeyServiceWorkerReply(const std::string& reply,
                                       std::string* user_id,
                                       std::string* input_key,
                                       std::string* model_id,
                                       std::string* model_key);

#endif  // SECURE_SERVERLESS_MESSAGE_WORKERKS_H_
