#ifndef SECURE_SERVERLESS_STORE_STORE_H_
#define SECURE_SERVERLESS_STORE_STORE_H_

#include <string>

class Store {
 public:
  virtual ~Store() = default;

  /**
   * @brief Get the content of a certain uid
   *
   * @param key the uid of the content to load from the storage service
   * @param value returned content
   * @return int 0 for success; -1 for failure
   */
  virtual int Get(const std::string& key, std::string* value) = 0;

  /**
   * @brief version that returns char array. (caller owns the buffer)
   *
   * @param key the uid of the content to load from the storage service
   * @param len return size
   * @return int 0 for success; -1 for failure
   */
  virtual char* Get(const std::string& key, size_t* len) = 0;

  /**
   * @brief Put the content under a certain uid
   *
   * @param key target uid in the storage service to assign to the content
   * @param value content to be stored
   * @return int 0 for success; -1 for failure
   */
  virtual int Put(const std::string& key, const std::string& value) = 0;

  /**
   * @brief close the storage service
   *
   * @return int 0 for success; -1 for failure
   */
  virtual int Close() = 0;
};

/**
 * @brief Create a FsStore instance.
 *  uid: relative path to the root path
 *
 * @param path root path of the store
 * @return Store* returned Store instance (caller free)
 */
Store* OpenFsStore(const std::string& path);

/**
 * @brief Create a FsStore instance.
 *  uid: relative path to the root path
 *
 * @param path root path of the store
 * @param cache_path cache path of the store
 * @param sim_remote_lat simulated remote store access latency (ms)
 * @return Store* returned Store instance (caller free)
 */
Store* OpenFsStore(const std::string& path, const std::string& cache_path,
                   int sim_remote_lat = 0);

/**
 * @brief Create a FsStore instance.
 *  uid: relative path to the root path
 *
 * @param path root path of the store
 * @return Store* returned Store instance (caller free)
 */
Store* OpenCORStore(const std::string& url_prefix);

#endif  // SECURE_SERVERLESS_STORE_STORE_H_
