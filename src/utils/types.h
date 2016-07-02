#ifndef TYPES_H_
#define TYPES_H_

// Basic types.
typedef char            int8_t;
typedef unsigned char   uint8_t;
typedef short           int16_t;
typedef unsigned short  uint16_t;
typedef int             int32_t;
typedef unsigned int    uint32_t;
typedef int32_t         ssize_t;
typedef uint32_t        size_t;

// Represents a raw physical address.
typedef long phys_addr_t;
// Represents a raw virtual address.
typedef long virt_addr_t;

// Represents the type of a variable that has no useful type.
// This is a useful type for linker script variables.
typedef long notype;

#define NULL 0

typedef int32_t         bool;
#define true  1
#define false 0

#endif // TYPES_H_
