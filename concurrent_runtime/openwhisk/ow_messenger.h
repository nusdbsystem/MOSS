/*
  Provides serverless platform dependent implementation of worker service
  message processing callbacks

  OpenWhisk action interface spec here:
  https://github.com/apache/openwhisk/blob/master/docs/actions-new.md
*/

#ifndef SECURE_SERVERLESS_CONCURRENTRT_U_OW_OWMESSENGER_H_
#define SECURE_SERVERLESS_CONCURRENTRT_U_OW_OWMESSENGER_H_

#include "common/json.h"
#include "platform_messenger.h"

class OwMessenger : public Messenger {
 public:
  //  openwhisk run request has a json body with value field
  //    containing the user passed in json for action invocation.
  inline std::string ExtractUserJson(
      const std::string& platform_message) const override {
    auto input = json::JSON::Load(platform_message);
    return input["value"].ToString();
  }

  //  openwhisk accept a json body for response.
  //    for error, it needs a json with a single field called "error"
  inline std::string PackageResponse(
      bool error, const std::string& response) const override {
    return error ? "{\"error\": \"" + std::move(response) + "\"}"
                 : "{\"msg\": \"" + std::move(response) + "\"}";
  }
};

#endif  // SECURE_SERVERLESS_CONCURRENTRT_U_OW_OWMESSENGER_H_
