#ifndef TYPES_H_
#define TYPES_H_

// Denotes that a function is defined in an assembly file.
#define __asm_def

// Denotes that an object goes in a non-standard section.
#define __section(section) __attribute__((__section__(section)))

// Denotes that a function goes in a pre-mmu-initialized text section.
#define __pre_mmu __section(".premmu.text")

#define __pre_mmu_data __section(".premmu.data")

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
