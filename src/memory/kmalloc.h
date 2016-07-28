#ifndef MEMORY_KMALLOC_H_
#define MEMORY_KMALLOC_H_

#include "memory/mmu.h"
#include "utils/types.h"

extern void* kmalloc(size_t size, uint32_t attrs);

extern void kfree(void* ptr);

#endif // MEMORY_KAMLLOC_H_
