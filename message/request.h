#ifndef SECURE_SERVERLESS_WORKER_REQUEST_H_
#define SECURE_SERVERLESS_WORKER_REQUEST_H_

#include <string>

class Message {
 public:
  virtual ~Message() = default;
  virtual std::string EncodeTo() const = 0;
};

typedef Message Request;
typedef Message Reply;

#endif  // SECURE_SERVERLESS_WORKER_REQUEST_H_
