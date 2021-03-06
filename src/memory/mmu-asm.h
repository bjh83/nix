#ifndef MEMORY_MMU_ASM_H_
#define MEMORY_MMU_ASM_H_

#include "utils/types.h"

#define RAM_START 0x80000000
#define RAM_SIZE  0x20000000
#define RAM_END   (RAM_START + RAM_SIZE)

#define STACK_SIZE 0x5000

#define PAGE_SHIFT                                12
#define PAGE_SIZE                           (1 << 12)
#define PAGE_MASK                   (~(PAGE_SIZE - 1))
#define PAGE_ADDR(addr)             ((addr) & PAGE_MASK)

#define PAGE_NOT_EXECUTABLE                 (1 << 0)
#define PAGE_SMALL                          (1 << 1)
#define PAGE_FAULT                                0
#define PAGE_TYPE_MASK         (((1 << 2) - 1) << 0)
#define PAGE_B                              (1 << 2)
#define PAGE_C                              (1 << 3)
#define PAGE_AP_MASK           (((1 << 2) - 1) << 4)
#define PAGE_TEX_MASK          (((1 << 3) - 1) << 6)
#define PAGE_TEXCB_DEVICE                   (PAGE_B)
#define PAGE_TEXCB_WRITE_NO_ALLOC  (PAGE_B | PAGE_C)
#define PAGE_AP_2                           (1 << 9)
#define PAGE_AP_RW_PL1                            0
#define PAGE_AP_RO_PL1                      (1 << 9)
#define PAGE_SHARABLE                       (1 << 10)
#define PAGE_NOT_GLOBAL                     (1 << 11)
#define PAGE_SIZE                           (1 << 12)
#define PAGE_MASK                   (~(PAGE_SIZE - 1))
#define PAGE_ADDR(addr)             ((addr) & PAGE_MASK)

#define PTAB_TYPE_MASK   (((1 << 2) - 1) << 0)
#define PTAB_FAULT                          0
#define PTAB_PTAB                     (1 << 0)
#define PTAB_NOT_EXECUTABLE           (1 << 2)
#define PTAB_NOT_SECURE               (1 << 3)
#define PTAB_DOMAIN_MASK (((1 << 4) - 1) << 5)
#define PTAB_IMPL_DEFN                (1 << 9)
#define PTAB_SIZE                     (1 << 10)
#define PTAB_MASK       (~(PTAB_SIZE - 1))
#define PTAB_ADDR(addr) ((addr) & PTAB_MASK)

#define TTAB_INDEX_SIZE     (1 << 12)
#define TTAB_INDEX_SHIFT          20
#define TTAB_INDEX_MASK     (TTAB_INDEX_SIZE - 1)
#define TTAB_INDEX(addr)    (((addr) & ((TTAB_INDEX_SIZE - 1) << TTAB_INDEX_SHIFT)) >> TTAB_INDEX_SHIFT)
#define TTAB_ADDR_ALIGNMENT (1 << 14)
#define TTAB_ADDR_MASK      (~(TTAB_ADDR_ALIGNMENT - 1))
#define PTAB_INDEX_SIZE     (1 << 8)
#define PTAB_INDEX_SHIFT          12
#define PTAB_INDEX_MASK     (PTAB_INDEX_SIZE - 1)
#define PTAB_INDEX(addr)    (((addr) & ((PTAB_INDEX_SIZE - 1) << PTAB_INDEX_SHIFT)) >> PTAB_INDEX_SHIFT)
#define PTAB_ADDR_ALIGNMENT (1 << 10)
#define PTAB_ADDR_MASK      (~(PTAB_ADDR_ALIGNMENT - 1))
#define PAGE_INDEX(addr)    ((addr) & (PAGE_SIZE - 1))

#define TTBR_CACHABLE              (1 << 0)
#define TTBR_SHARABLE              (1 << 1)
#define TTBR_IMP                   (1 << 2)
#define TTBR_RGN_MASK (((1 << 2) - 1) << 3)
#define TTBR_NOS                   (1 << 5)
#define TTBR_TTAB_MASK TTAB_ADDR_MASK

#define TTBCR_N_MASK  (((1 << 3) - 1) << 0)
#define TTBCR_USE_TTBR0                  0
#define TTBCR_PD0                  (1 << 4)
#define TTBCR_PD1                  (1 << 5)

typedef long tran_table_t;
typedef long page_table_t;
typedef uint32_t page_entry_t;

#endif // MEMORY_MMU_ASM_H_
