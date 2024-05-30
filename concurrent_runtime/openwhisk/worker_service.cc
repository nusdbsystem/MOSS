#include "worker_service.h"

#include <cassert>
#include <chrono>
#include <climits>  // for INT_MAX
#include <cstring>  // for strlen
#include <thread>

// implementation of ocalls
void ocall_debug_print(const void* s, size_t len) {
  assert(len < INT_MAX);
  printf("DEBUG PRINT: %.*s\n", (int)len, (const char*)s);
}
void ocall_debug_print_string(const char* s) { printf("DEBUG PRINT: %s\n", s); }
void ocall_debug_print_hexstring(const char* s) {
  printf("DEBUG PRINT (hex): ");
  for (unsigned int i = 0; i < strlen(s); i++) {
    printf("%02hhx", (unsigned char)s[i]);
  }
  printf("\n");
}
void ocall_debug_print_hex(const void* s, size_t len) {
  printf("DEBUG PRINT (hex): ");
  auto it = (const unsigned char*)s;
  for (unsigned int i = 0; i < len; i++) {
    printf("%02hhx", *(it++));
  }
  printf("\n");
}
// implementation of ocalls

void initialize_once(Worker* worker, response_code_t* code) {
  // execute logic on the value for initialization
  // decode json request
  // return 404 for decode failure
  printf("init execution\n");
  // execute logic on the value for initialization
  *code = worker->Initialize() ? OK : SERVER_ERROR;
}

response_code_t WorkerService::Init(const std::string& /*request*/,
                                    std::string* response) {
  assert(response);
  response_code_t code = FORBIDDEN;

  std::call_once(initalized_, initialize_once, &worker_, &code);
  switch (code) {
    case FORBIDDEN:
      // send response of error 403
      printf("init phase: reinitalize forbidden by default\n");
      *response = "reinit forbidden";
      break;
    case OK:
      *response = "init finished";
      code = OK;
      break;
    default:
      *response = "unkown server code from init";
      code = SERVER_ERROR;
      break;
  }

  return code;
}

response_code_t WorkerService::Run(const std::string& request,
                                   std::string* response) {
  assert(response);

  response_code_t code = OK;
  std::call_once(initalized_, initialize_once, &worker_, &code);
  if (code != OK) {
    // called init and errors encountered
    *response = "init during run and error encountered";
    return SERVER_ERROR;
  }
  printf("run execution");

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // upon error return 502 (500 can be used for framework internal error)

  // decode the request
  // execute the logic
  std::string prediction;
  auto ret = worker_.Handle(0, request, &prediction);
  if (ret != 0) {
    // *response = "worker execution error";
    *response = prediction;
    return SERVER_ERROR;
  }

  // wrap the result into a response json
  *response = std::move(prediction);

  // return 200 with the response
  return OK;
}
