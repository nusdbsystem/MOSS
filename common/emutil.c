#include "emutil.h"

#include "assert.h"

char* emem_getline(emem* m) {
  if (m->curr >= (m->buf + m->size)) return NULL;
  const char* tmp = strchr(m->curr, '\n');
  // reached the end
  if (tmp == NULL) tmp = m->buf + m->size;

  size_t to_copy = tmp - m->curr;
  char* line = (char*)malloc(to_copy * sizeof(char) + 1);
  if (line == NULL) return NULL;  // malloc failed
  memcpy(line, m->curr, to_copy);
  line[to_copy] = '\0';

  // advance m->curr
  m->curr +=
      ((m->curr + to_copy) == (m->buf + m->size))
          ? to_copy
          : to_copy + 1;  // skip the newline character if not at the end.
  return line;
}

size_t emem_read(void* dest, size_t size, emem* m) {
  size_t to_copy =
      ((m->curr + size) > (m->buf + m->size)) ? (m->buf - m->curr) : size;
  assert(dest != NULL);
  memcpy(dest, m->curr, to_copy);
  m->curr += to_copy;
  return to_copy;
}

bool emem_reached_end(const emem* m) { return m->curr >= m->buf + m->size; }
