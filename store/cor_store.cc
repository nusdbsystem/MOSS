#include "cor_store.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

#include "curl/curl.h"

namespace {

struct buffer_and_size {
  char* data;
  size_t len;
};

size_t accumulate_function(void* ptr, size_t size, size_t nmemb,
                           void* userdata) {
  struct buffer_and_size* s = (struct buffer_and_size*)userdata;
  s->data = (char*)realloc(s->data, s->len + size * nmemb);
  assert(s->data != NULL);
  memcpy(s->data + s->len, ptr, size * nmemb);
  s->len += size * nmemb;
  return size * nmemb;
}

void http_get(CURL* curl, const char* url, struct buffer_and_size* header,
              struct buffer_and_size* body) {
  curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
  curl_easy_setopt(curl, CURLOPT_URL, url);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L);

  curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, accumulate_function);
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, header);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, accumulate_function);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, body);

  CURLcode res = curl_easy_perform(curl);

  assert(res == CURLE_OK);
}

inline std::string craft_link(const std::string prefix, const std::string key) {
  return "http://" + prefix + "/" + key;
}
}  // anonymous namespace

int CORStore::Get(const std::string& key, std::string* value) {
  assert(value);
  size_t body_len;
  auto ret = Get(key, &body_len);
  value->assign(ret, body_len);
  free(ret);
  return 0;
}

char* CORStore::Get(const std::string& key, size_t* len) {
  CURL* curl = curl_easy_init();
  assert(curl);
  auto url = craft_link(url_prefix_, std::move(key));

  struct buffer_and_size header = {(char*)malloc(1), 0};
  struct buffer_and_size body = {(char*)malloc(1), 0};
  http_get(curl, url.c_str(), &header, &body);
  free(header.data);
  *len = body.len;
  curl_easy_cleanup(curl);
  return body.data;
}

// no caching, assuming serverless instance stateless
int CORStore::Put(const std::string& /*key*/, const std::string& /*value*/) {
  // no-op
  return -1;
}

Store* OpenCORStore(const std::string& url_prefix) {
  return new CORStore(std::move(url_prefix));
}
