#ifndef SECURE_SERVERLESS_STORE_FSSTORE_H_
#define SECURE_SERVERLESS_STORE_FSSTORE_H_

#include "store.h"

/**
 * @brief An Store implemntation that stores the key value pairs as files
 * on local file system.
 *
 * Explanation: the distinction of cache path and path is to work with
 *  mounted paths which can have higher latency to access. The cache path
 *  can be a fast path such as /tmp.
 *
 * cache_path_: when not empty, the accessed files are copied over from
 *  path to cache_path_
 * path_: the path to load content from.
 */

class FsStore : public Store {
 public:
  FsStore() = delete;
  ~FsStore() { Close(); };
  FsStore(const std::string& path)
      : cache_path_(""), path_(std::move(path)), sim_remote_lat_(0) {}
  FsStore(const std::string& path, const std::string& cache_path,
          int sim_remote_lat = 0)
      : cache_path_((cache_path.compare(path)) ? std::move(cache_path) : ""),
        path_(std::move(path)),
        sim_remote_lat_(sim_remote_lat) {}

  FsStore(const FsStore&) = delete;
  FsStore(FsStore&&) = delete;
  FsStore& operator=(const FsStore&) = delete;
  FsStore& operator=(FsStore&&) = delete;

  int Get(const std::string& key, std::string* value) override;

  char* Get(const std::string& key, size_t* len) override;

  int Put(const std::string& key, const std::string& value) override;

  int Close() override { /*no_op*/ return 0; };

 private:
  const std::string cache_path_;
  const std::string path_;
  const int sim_remote_lat_;
};

#endif  // SECURE_SERVERLESS_STORE_FSSTORE_H_
