#ifndef SECURE_SERVERLESS_MESSAGE_CHANNELSERVER_H_
#define SECURE_SERVERLESS_MESSAGE_CHANNELSERVER_H_

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <atomic>
#include <vector>

#include "Enclave_u.h"
#include "channel.h"

class Service {
 public:
  virtual ~Service() = default;
  virtual int Handle(WOLFSSL* ssl) = 0;
};

class ChannelServer {
 public:
  ChannelServer(uint16_t port, const CheckContext& check_ctx,
                int allowed_connection)
      : initialized_(false),
        serving_(false),
        checker_(check_ctx),
        svc_(nullptr),
        sockfd_(-1),
        port_(port),
        allowed_connection_(allowed_connection) {}
  virtual ~ChannelServer() = default;

  virtual int Initialize();

  inline bool RegisterService(Service* svc) {
    if (svc_) return false;
    svc_ = svc;
    return true;
  }

  virtual bool Serve() = 0;
  virtual int Close() = 0;

  inline bool IsInitialized() const { return initialized_; }

 protected:
  bool initialized_;
  bool serving_;
  const CheckContext& checker_;
  Service* svc_;

  int sockfd_;
  uint16_t port_;
  sockaddr_in addr_;
  int allowed_connection_;
};

class RatlsChannelServer : public ChannelServer {
 public:
  RatlsChannelServer(sgx_enclave_id_t id, uint16_t port,
                     const CheckContext& check_ctx, bool enclave_client,
                     int allowed_connection)
      : ChannelServer(port, check_ctx, allowed_connection),
        id_(id),
        enclave_client_(enclave_client),
        ctx_(nullptr),
        error_flag_(false) {}
  ~RatlsChannelServer() { Close(); }

  RatlsChannelServer(const RatlsChannelServer&) = delete;
  RatlsChannelServer& operator=(const RatlsChannelServer&) = delete;
  RatlsChannelServer(RatlsChannelServer&&) = delete;
  RatlsChannelServer& operator=(RatlsChannelServer&&) = delete;

  int Initialize();
  bool Serve() override;
  int Close() override;

 private:
  int SetVerify();

  sgx_enclave_id_t id_;
  bool enclave_client_;
  WOLFSSL_CTX* ctx_;
  std::atomic_bool error_flag_;
};

// // not used
// class NormalChannelServer : public ChannelServer {
//  public:
//   NormalChannelServer(uint16_t port, const CheckContext& check_ctx,
//     uint32_t buffer_size = BUFSIZ)
//     : ChannelServer(port, check_ctx, buffer_size){}

//   ~NormalChannelServer() { Close(); }

//   NormalChannelServer(const NormalChannelServer&) = delete;
//   NormalChannelServer& operator=(const NormalChannelServer&) = delete;
//   NormalChannelServer(NormalChannelServer&&) = delete;
//   NormalChannelServer& operator=(NormalChannelServer&&) = delete;

//   int Initialize() override;

//   int Connect() override;

//   int Close() override;

//   int Read(std::string* output) override;

//   int Send(const std::string& msg) override;

//   int CloseConnection() override;

//  private:
//   int SetVerify() override { /*no op*/ return 0; }
// };

#endif  // SECURE_SERVERLESS_MESSAGE_CHANNELSERVER_H_
