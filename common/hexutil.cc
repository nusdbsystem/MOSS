/*

Copyright 2018 Intel Corporation

This software and the related documents are Intel copyrighted materials,
and your use of them is governed by the express license under which they
were provided to you (License). Unless the License provides otherwise,
you may not use, modify, copy, publish, distribute, disclose or transmit
this software or the related documents without Intel's prior written
permission.

This software and the related documents are provided as is, with no
express or implied warranties, other than those that are expressly stated
in the License.

*/

#include "hexutil.h"

#include <cstdint>
#include <cstdlib>
#include <memory>

namespace {
const char _hextable[] = "0123456789abcdef";

struct hex_buffer_deleter {
  void operator()(void *p) const { free(p); }
};

thread_local std::unique_ptr<char, hex_buffer_deleter> _hex_buffer{nullptr};
thread_local size_t _hex_buffer_size = 0;
}  // anonymous namespace

void print_hexstring(FILE *fp, const void *vsrc, size_t len) {
  const unsigned char *sp = (const unsigned char *)vsrc;
  size_t i;
  for (i = 0; i < len; ++i) {
    fprintf(fp, "%02x", sp[i]);
  }
  fprintf(fp, "\n");
}

const char *hexstring(const void *vsrc, size_t len) {
  size_t i, bsz;
  const char *src = (const char *)vsrc;
  char *bp;

  bsz = len * 2 + 1; /* Make room for NULL byte */
  if (bsz >= _hex_buffer_size) {
    /* Allocate in 1K increments. Make room for the NULL byte. */
    size_t newsz = 1024 * (bsz / 1024) + ((bsz % 1024) ? 1024 : 0);
    _hex_buffer_size = newsz;
    auto _hex_buffer_ptr = (char *)realloc(_hex_buffer.get(), newsz);
    if (_hex_buffer_ptr == NULL) {
      return "(out of memory)";
    }
    _hex_buffer.release();
    _hex_buffer.reset(_hex_buffer_ptr);
  }

  for (i = 0, bp = _hex_buffer.get(); i < len; ++i) {
    *bp = _hextable[(uint8_t)src[i] >> 4];
    ++bp;
    *bp = _hextable[(uint8_t)src[i] & 0xf];
    ++bp;
  }
  _hex_buffer.get()[len * 2] = 0;

  return (const char *)_hex_buffer.get();
}
