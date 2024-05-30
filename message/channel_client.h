#ifndef SECURE_SERVERLESS_MESSAGE_CHANNELCLIENT_H_
#define SECURE_SERVERLESS_MESSAGE_CHANNELCLIENT_H_

#include "channel.h"
#include "wolfssl/ssl.h"

class RatlsChannelClient : public Channel {
 public:
  RatlsChannelClient(uint16_t port, const std::string& server_addr,
                     const CheckContext& check_ctx)
      : initialized_(false),
        connected_(false),
        sockfd_(-1),
        checker_(check_ctx),
        ctx_(nullptr),
        port_(port),
        server_addr_(server_addr),
        ssl_(nullptr) {}

  ~RatlsChannelClient() { Close(); }

  RatlsChannelClient(const RatlsChannelClient&) = delete;
  RatlsChannelClient& operator=(const RatlsChannelClient&) = delete;
  RatlsChannelClient(RatlsChannelClient&&) = delete;
  RatlsChannelClient& operator=(RatlsChannelClient&&) = delete;

  int Initialize();

  int Connect();

  int CloseConnection() override;

  int Close();

  int Read(std::string* output) override;

  int Send(const std::string& msg) override;

  inline bool IsInitialized() const { return initialized_; }
  inline bool IsConnected() const { return connected_; }

 private:
  int SetVerify();

  bool initialized_;
  bool connected_;
  int sockfd_;
  const CheckContext& checker_;
  WOLFSSL_CTX* ctx_;

  uint16_t port_;
  const std::string server_addr_;
  WOLFSSL* ssl_;
};

#endif  // SECURE_SERVERLESS_MESSAGE_CHANNELCLIENT_H_
