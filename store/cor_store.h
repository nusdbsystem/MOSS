#ifndef SECURE_SERVERLESSS_STORE_CORSTORE_H_
#define SECURE_SERVERLESSS_STORE_CORSTORE_H_

#include "store.h"

/**
 * @brief An Store implemntation that stores the key value pairs as cloud object
 * storage (now only read from http endpoint)
 *
 */

class CORStore : public Store {
 public:
  CORStore() = delete;
  ~CORStore() = default;

  CORStore(const std::string& url_prefix)
      : url_prefix_(std::move(url_prefix)) {}

  CORStore(const CORStore&) = delete;
  CORStore(CORStore&&) = delete;
  CORStore& operator=(const CORStore&) = delete;
  CORStore& operator=(CORStore&&) = delete;

  int Get(const std::string& key, std::string* value) override;

  char* Get(const std::string& key, size_t* len) override;

  // no caching, assuming serverless instance stateless
  int Put(const std::string& key, const std::string& value) override;

  int Close() override { /*no_op*/ return 0; };

 private:
  const std::string url_prefix_;
};

#endif  // SECURE_SERVERLESSS_STORE_CORSTORE_H_
