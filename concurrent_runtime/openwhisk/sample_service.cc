#include "sample_service.h"

#include <cassert>
#include <chrono>
#include <thread>

response_code_t SampleService::Init(const std::string& /*request*/,
                                    std::string* response) {
  assert(response);
  if (initialized_) {
    // send response of error 403
    printf("init phase: reinitalize forbidden by default\n");
    *response = "reinit forbidden";
    return FORBIDDEN;
  }
  // decode json request
  // return 404 for decode failure
  printf("init execution\n");

  // execute logic on the value for initialization
  initialized_ = true;
  *response = "init finished (sample)";
  return OK;
}

response_code_t SampleService::Run(const std::string& request,
                                   std::string* response) {
  printf("\nUser request:\n%s\n", (request.empty()) ? "" : request.c_str());

  if (!initialized_) {
    printf("service not yet initialized: Initializing now\n");
    *response = "not yet initialized (sample)";
    return SERVER_ERROR;
  }
  assert(response);
  printf("run execution on user input: \n%s\n",
         (request.empty()) ? "" : request.c_str());

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  // upon error return 502 (500 can be used for framework internal error)

  // decode the request

  // execute the logic

  // wrap the result into a response json

  // return 200 with the response
  *response = "run finished (sample)";
  return OK;
}
