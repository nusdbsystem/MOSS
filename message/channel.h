#ifndef SECURE_SERVERLESS_MESSAGE_CHANNEL_H_
#define SECURE_SERVERLESS_MESSAGE_CHANNEL_H_

#define RCVBUFSIZE 4096

#include <memory>
#include <string>

struct CheckContext {};

class Channel {
 public:
  virtual ~Channel() = default;
  virtual int CloseConnection() = 0;
  virtual int Read(std::string* output) = 0;
  virtual int Send(const std::string& msg) = 0;
};

#endif  // SECURE_SERVERLESS_MESSAGE_CHANNEL_H_
