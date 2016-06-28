#ifndef UTILS_ERR_H_
#define UTILS_ERR_H_

#include "utils/errno.h"
#include "utils/types.h"

inline void* ERR_PTR(long err) {
  return (void*) err;
}

inline long PTR_ERR(const void* ptr) {
  return (long) ptr;
}

inline bool IS_ERR(const void* ptr) {
  return PTR_ERR(ptr) >= -MAX_ERRNO;
}

inline bool IS_NULL_OR_ERR(const void* ptr) {
  return !ptr || IS_ERR(ptr);
}

int ZERO_OR_ERR(const void* ptr) {
  if (IS_ERR(ptr)) {
    return PTR_ERR(ptr);
  } else {
    return 0;
  }
}

#endif // UTILS_ERR_H_
