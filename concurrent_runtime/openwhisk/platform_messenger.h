/*
  Inferface for serverless platform dependent implementation message processing
*/

#ifndef SECURE_SERVERLESS_CONCURRENTRT_U_OW_PLATFORMMESSENGER_H_
#define SECURE_SERVERLESS_CONCURRENTRT_U_OW_PLATFORMMESSENGER_H_

#include <string>

class Messenger {
 public:
  virtual ~Messenger() = default;

  // extract users request json from the serverless platform request
  virtual std::string ExtractUserJson(
      const std::string& platform_message) const = 0;

  // package worker result to serverless platform compatible response format
  virtual std::string PackageResponse(bool error,
                                      const std::string& result) const = 0;
};

#endif  // SECURE_SERVERLESS_CONCURRENTRT_U_OW_PLATFORMMESSENGER_H_
