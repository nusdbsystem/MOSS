#include "Enclave_u.h"
#include "sgx_urts.h"

#define KEYSERVER_ENCLAVE_FILENAME "KeyServer_Enclave.signed.so"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cassert>
#include <string>
#include <thread>

#include "message/channel_server.h"
#include "store/store.h"

#define MAXRCVDATASIZE 2 * 1024 * 1024

#define KEYSERVICE_CLIENT_PORT 13570
#define KEYSERVICE_WORKER_PORT 13571
#define CON_CLIENT_CONNECT 2
#define CON_WORKER_CONNECT 16

class ClientService : public Service {
 public:
  ClientService(sgx_enclave_id_t id, Store* store) : id_(id), store_(store) {}
  inline int Handle(WOLFSSL* ssl) override {
    sgx_status_t retval = SGX_SUCCESS;
    if ((enc_client_service(id_, &retval, ssl, store_) != SGX_SUCCESS) ||
        retval != SGX_SUCCESS) {
      printf("failure in serving client service\n");
      return -1;
    }
    return 0;
  }

 private:
  sgx_enclave_id_t id_;
  Store* store_;
};

class WorkerService : public Service {
 public:
  WorkerService(sgx_enclave_id_t id, Store* store) : id_(id), store_(store) {}
  inline int Handle(WOLFSSL* ssl) override {
    sgx_status_t retval = SGX_SUCCESS;
    if ((enc_worker_service(id_, &retval, ssl, store_) != SGX_SUCCESS) ||
        retval != SGX_SUCCESS) {
      printf("failure in serving worker service\n");
      return -1;
    }
    return 0;
  }

 private:
  sgx_enclave_id_t id_;
  Store* store_;
};

int ServeClient(sgx_enclave_id_t id, Store* store, int allowed_connection) {
  assert(store);
  CheckContext ctx;
  ClientService svc{id, store};
  RatlsChannelServer server{id, KEYSERVICE_CLIENT_PORT, ctx, false,
                            allowed_connection};
  if (server.Initialize() == -1) {
    printf("Failed to Initialize ratls channel\n");
    return -1;
  }
  server.RegisterService(&svc);
  int ret = server.Serve();
  server.Close();
  return ret;
}

int ServeWorker(sgx_enclave_id_t id, Store* store, int allowed_connection) {
  assert(store);
  CheckContext ctx;
  WorkerService svc{id, store};
  RatlsChannelServer server{id, KEYSERVICE_WORKER_PORT, ctx, true,
                            allowed_connection};
  if (server.Initialize() == -1) {
    printf("Failed to Initialize ratls channel\n");
    return -1;
  }
  server.RegisterService(&svc);
  int ret = server.Serve();
  server.Close();
  return ret;
}

int main(int argc, char* argv[]) {
  int ret = 0;

  std::string keyserver_enclave_filename = KEYSERVER_ENCLAVE_FILENAME;

  if (argc == 3) {
    keyserver_enclave_filename = argv[2];
  } else if (argc != 2) {
    fprintf(stderr, "Usage: store_path [signed-enclave-shared-object]\n");
    exit(1);
  }

  std::string store_path(argv[1]);

  sgx_enclave_id_t id;
  sgx_launch_token_t t;

  // initialize worker enclave
  int updated = 0;
  memset(t, 0, sizeof(sgx_launch_token_t));
  ret = sgx_create_enclave(keyserver_enclave_filename.c_str(), SGX_DEBUG_FLAG,
                           &t, &updated, &id, NULL);
  if (ret != SGX_SUCCESS) {
    printf("Failed to create Enclave : error %d - %#x.\n", ret, ret);
    return 1;
  } else
    printf("Enclave launched with id: %ld.\n", id);

  Store* store = OpenFsStore(store_path);

  std::thread client_service =
      std::thread(ServeClient, id, store, CON_CLIENT_CONNECT);
  std::thread worker_service =
      std::thread(ServeWorker, id, store, CON_WORKER_CONNECT);

  printf("both service launched\n");

  client_service.join();
  worker_service.join();

  delete store;

  return 0;
}
