#ifndef MEMORY_MMU_H_
#define MEMORY_MMU_H_

#include "memory/mmu-asm.h"
#include "utils/types.h"

#define MEM_EXEC        (1 << 0)
#define MEM_WRITE       (1 << 1)
#define MEM_CACHE       (1 << 2)
#define MEM_IO          (1 << 3)

#define MEM_EXEC_NORMAL (MEM_CACHE | MEM_EXEC)
#define MEM_DATA_NORMAL (MEM_CACHE | MEM_WRITE)

static inline virt_addr_t phys_to_virt(phys_addr_t phys) {
  return phys - RAM_START;
}

static inline phys_addr_t virt_to_phys(virt_addr_t virt) {
  return virt + RAM_START;
}

static inline void* __va(phys_addr_t paddr) {
  return (void*) phys_to_virt(paddr);
}

static inline phys_addr_t __pa(void* ptr) {
  return virt_to_phys((virt_addr_t) ptr);
}

static inline page_table_t get_page_table(tran_table_t ttab, uint32_t index) {
  phys_addr_t phys = (ttab & TTAB_ADDR_MASK) | ((index & TTAB_INDEX_MASK) << 2);
  virt_addr_t virt = phys_to_virt(phys);
  return *((page_table_t*) virt);
}

static inline void set_page_table(tran_table_t ttab, uint32_t index, page_table_t ptab) {
  phys_addr_t phys = (ttab & TTAB_ADDR_MASK) | ((index & TTAB_INDEX_MASK) << 2);
  virt_addr_t virt = phys_to_virt(phys);
  *((page_table_t*) virt) = ptab;
}

static inline page_entry_t get_page_entry(page_table_t ptab, uint32_t index) {
  phys_addr_t phys = (ptab & PTAB_ADDR_MASK) | ((index & PTAB_INDEX_MASK) << 2);
  virt_addr_t virt = phys_to_virt(phys);
  return *((page_entry_t*) virt);
}

static inline void set_page_entry(page_table_t ptab, uint32_t index, page_entry_t entry) {
  phys_addr_t phys = (ptab & PTAB_ADDR_MASK) | ((index & PTAB_INDEX_MASK) << 2);
  virt_addr_t virt = phys_to_virt(phys);
  *((page_table_t*) virt) = entry;
}

extern tran_table_t kernel_ttab;

extern page_entry_t lookup_page(tran_table_t ttab, void* page_addr);
extern int assign_page(tran_table_t ttab, void* page_addr, page_entry_t page_entry);
extern void flush_mem(void* mem, size_t size);

extern void* page_alloc(size_t page_num, uint32_t attrs);
extern void page_free(void* page, size_t page_num);
extern int set_page_attrs(virt_addr_t page_addr, uint32_t attrs);

extern phys_addr_t  __asm_def __translate_addr(virt_addr_t addr);

extern void __asm_def __flush_page(virt_addr_t vaddr);

static inline void flush_page(virt_addr_t vaddr) {
  __flush_page(vaddr);
}

extern void __asm_def __set_ttbr0(uint32_t new_ttbr0);
extern phys_addr_t __asm_def __get_ttbr0(void);
extern void __asm_def __set_ttbcr(uint32_t new_ttbcr);
extern void __asm_def __set_dacr(uint32_t new_dacr);
extern void __asm_def __disable_irqs(void);

#endif // MEMORY_MMU_H_
