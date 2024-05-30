#ifndef SECURE_SERVERLESS_CONCURRENTRT_U_OW_SAMPLESERVICE_H_
#define SECURE_SERVERLESS_CONCURRENTRT_U_OW_SAMPLESERVICE_H_

#include "service.h"

class SampleService : public Service {
 public:
  SampleService() : initialized_(false) {}
  ~SampleService() = default;

  response_code_t Init(const std::string& request,
                       std::string* response) override;

  response_code_t Run(const std::string& request,
                      std::string* response) override;

 private:
  bool initialized_;
};

#endif  // SECURE_SERVERLESS_CONCURRENTRT_U_OW_SAMPLESERVICE_H_
