#ifndef UTILS_BASE_H_
#define UTILS_BASE_H_

#include "utils/types.h"

extern uint32_t be_to_le(uint32_t big_endian);

extern size_t strlen(const char* str);

extern int putchar_to_puts(int (*putchar)(char), const char* str);
extern int putchar_to_printf(int (*putchar)(char), const char* format, ...);

// Converts a value to a string representation in the specified radix.
// Assumes base is between 2 and 36.
// Assumes buf is of length 33 or greater (size required to represent an
// unsigned 32-bit number in binary with a null-terminator at the end).
extern char* itoa(int value, char* buf, int base);

extern int __aeabi_idivmod(int left, int right);
extern int __aeabi_idiv(int left, int right);

#endif // UTILS_BASE_H_
