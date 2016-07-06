#ifndef UTILS_ERR_H_
#define UTILS_ERR_H_

#include "utils/errno.h"
#include "utils/types.h"

static inline void* ERR_PTR(long err) {
  return (void*) err;
}

static inline long PTR_ERR(const void* ptr) {
  return (long) ptr;
}

static inline bool IS_ERR(long err) {
  return err >= -MAX_ERRNO;
}

static inline bool IS_PTR_ERR(const void* ptr) {
  return IS_ERR(PTR_ERR(ptr));
}

static inline bool IS_NULL_OR_ERR(const void* ptr) {
  return !ptr || IS_PTR_ERR(ptr);
}

static inline int ZERO_OR_ERR(const void* ptr) {
  if (IS_PTR_ERR(ptr)) {
    return PTR_ERR(ptr);
  } else {
    return 0;
  }
}

#endif // UTILS_ERR_H_
