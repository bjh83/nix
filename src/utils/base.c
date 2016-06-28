#include "utils/base.h"

#include <stdarg.h>

// Doesn't print anything or try to reset the system and is thus safe to use in
// core libraries.
#define BASE_ASSERT(cond) \
  if (!(cond)) {          \
    while (1);            \
  }

size_t strlen(const char* str) {
  size_t size;
  for (size = 0; str[size] != '\0'; size++);
  return size;
}

void* memcpy(void* destination, const void* source, size_t num) {
  char* dest = (char*) destination;
  const char* src = (const char*) source;
  for (size_t i = 0; i < num; i++) {
    dest[i] = src[i];
  }
  return destination;
}

void* memset(void* destination, int val, size_t num) {
  char* dest = (char*) destination;
  for (size_t i = 0; i < num; i++) {
    dest[i] = val;
  }
  return destination;
}

int max(int left, int right) {
  if (left > right) {
    return left;
  } else {
    return right;
  }
}

int sign(int value) {
  if (value < 0) {
    return -1;
  } else if (value > 0) {
    return 1;
  } else {
    return 0;
  }
}

int abs(int value) {
  return value * sign(value);
}

uint32_t be_to_le(uint32_t big_endian) {
  uint8_t* big_endian_ptr = (uint8_t*) &big_endian;
  uint32_t little_endian;
  uint8_t* little_endian_ptr = (uint8_t *) &little_endian;
  little_endian_ptr[3] = big_endian_ptr[0];
  little_endian_ptr[2] = big_endian_ptr[1];
  little_endian_ptr[1] = big_endian_ptr[2];
  little_endian_ptr[0] = big_endian_ptr[3];
  return little_endian;
}

// Converts a value to a string representation in the specified radix.
// Assumes base is between 2 and 36.
// Assumes buf is of length 33 or greater (size required to represent an
// unsigned 32-bit number in binary with a null-terminator at the end).
char* itoa(int value, char* buf, int base) {
  if (base < 1 || base > 36) {
    return NULL;
  }

  uint32_t uvalue;
  if (base == 10) {
    uvalue = abs(value);
  } else {
    uvalue = value;
  }
  const int buflen = 33;
  int i = buflen - 1;
  buf[i] = '\0';
  do {
    i--;

    const char digit = uvalue % base;
    if (digit <= 9) {
      buf[i] = '0' + digit;
    } else {
      buf[i] = 'a' + digit - 10;
    }

    uvalue /= base;
  } while (uvalue != 0 && i > 0);
  if (value < 0 && base == 10) {
    buf[--i] = '-';
  }
  return &buf[i];
}

int putchar_to_puts(int (*putchar)(char), const char* str) {
  for (size_t i = 0; i < strlen(str); i++) {
    if ((putchar)(str[i]) < 0) {
      return -1;
    }
  }
  return 0;
}

int putchar_to_printf(int (*putchar)(char), const char* format, ...) {
  size_t len = strlen(format);
  char numbuf[33];
  int ret = 0;
  va_list args;
  va_start(args, format);
  for (size_t i = 0; i < len; i++) {
    if (format[i] == '%' && i + 1 < len) {
      char* number;
      switch (format[++i]) {
        case 'i':
        case 'd':
          number = itoa(va_arg(args, int), numbuf, 10);
          ret = putchar_to_puts(putchar, number);
          if (ret < 0) {
            goto out;
          }
          break;
        case 'x':
          number = itoa(va_arg(args, int), numbuf, 16);
          ret = putchar_to_puts(putchar, number);
          if (ret < 0) {
            goto out;
          }
          break;
        case 'p': {
          number = itoa(va_arg(args, int), numbuf, 16);
          ret = putchar_to_puts(putchar, "0x");
          if (ret < 0) {
            goto out;
          }
          for (size_t i = 0; i < 8 - strlen(number); i++) {
            ret = (putchar)('0');
            if (ret < 0) {
              goto out;
            }
          }
          ret = putchar_to_puts(putchar, number);
          if (ret < 0) {
            goto out;
          }
          break;
        }
        case 's':
          ret = putchar_to_puts(putchar, va_arg(args, char*));
          if (ret < 0) {
            goto out;
          }
          break;
      }
    } else {
      ret = (putchar)(format[i]);
      if (ret < 0) {
        goto out;
      }
    }
  }
out:
  va_end(args);
  return ret;
}

static uint32_t unsigned_divide(uint32_t left, uint32_t right) {
  int shift;
  for (shift = 0; left > (right << shift) && !(0x80000000 & (right << shift)); shift++);
  uint32_t remainder = left;
  uint32_t result = 0;
  while (remainder >= right) {
    if (remainder < (right << shift)) {
      shift--;
      continue;
    }
    remainder -= (right << shift);
    result += (1 << shift);
  }
  return result;
}

unsigned int __aeabi_uidiv(unsigned int left, unsigned int right) {
  return unsigned_divide(left, right);
}

int __aeabi_idiv(int left, int right) {
  if ((left < 0 && right < 0) || (left > 0 && right > 0)) {
    return unsigned_divide(abs(left), abs(right));
  } else {
    return unsigned_divide(abs(left), abs(right)) * -1;
  }
}

char* itoa_hex(int value, char* str) {
  int i = 0;
  const int mask = 0xf;
  for (int shift = 32 - 4; shift >= 0; shift -= 4) {
    int digit = (value >> shift) & mask;
    if ((0x0 < digit || i > 1) && digit < 0x9) {
      str[i++] = '0' + digit;
    } else if (digit >= 0xa) {
      str[i++] = 'a' + digit;
    }
  }
  if (i == 0) {
    str[i++] = 0;
  }
  str[i] = '\0';
  return str;
}
