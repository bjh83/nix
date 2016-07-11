#include "memory/mmu-asm.h"
#include "utils/types.h"

// extern phys_addr_t __stack_bottom;
static phys_addr_t next_alloc_addr = 0x80500000;
static phys_addr_t kernel_ttab_paddr = 0xffffffff;

extern void __asm_def __premmu_set_ttbr0(uint32_t new_ttbr0);
extern void __asm_def __premmu_set_ttbcr(uint32_t new_ttbcr);
extern void __asm_def __premmu_set_dacr(uint32_t new_dacr);

static inline virt_addr_t phys_to_virt(phys_addr_t phys) {
  return phys - RAM_START;
}

static inline phys_addr_t virt_to_phys(virt_addr_t virt) {
  return virt + RAM_START;
}

static void* early_mmu_malloc(size_t size, uint32_t alignment) {
  // if (next_alloc_addr == 0) {
  //   next_alloc_addr = (phys_addr_t) &__stack_bottom;
  // }

  uint32_t mask = ~(alignment - 1);
  if ((next_alloc_addr & mask) != next_alloc_addr) {
    next_alloc_addr = (next_alloc_addr & mask) + alignment;
  }
  void* ret = (void *) next_alloc_addr;
  next_alloc_addr += size;
  return ret;
}

static page_table_t early_get_page_table(tran_table_t ttab, uint32_t index) {
  phys_addr_t phys = (ttab & TTAB_ADDR_MASK) | ((index & TTAB_INDEX_MASK) << 2);
  return *((page_table_t*) phys);
}

static void early_set_page_table(tran_table_t ttab, uint32_t index, page_table_t ptab) {
  phys_addr_t phys = (ttab & TTAB_ADDR_MASK) | ((index & TTAB_INDEX_MASK) << 2);
  *((page_table_t*) phys) = ptab;
}

static page_entry_t early_get_page_entry(page_table_t ptab, uint32_t index) {
  phys_addr_t phys = (ptab & PTAB_ADDR_MASK) | ((index & PTAB_INDEX_MASK) << 2);
  return *((page_entry_t*) phys);
}

static void early_set_page_entry(page_table_t ptab, uint32_t index, page_entry_t pentry) {
  phys_addr_t phys = (ptab & PTAB_ADDR_MASK) | ((index & PTAB_INDEX_MASK) << 2);
  *((page_entry_t*) phys) = pentry;
}

extern void __start_mmu(void) __attribute__((aligned(PAGE_SIZE), noreturn));

static phys_addr_t start_mmu_paddr;

static int flat_remap_paddr(tran_table_t ttab, phys_addr_t paddr) {
  page_table_t ptab = early_get_page_table(ttab, TTAB_INDEX(paddr));
  if (ptab == PTAB_FAULT) {
    phys_addr_t ptab_addr = (phys_addr_t) early_mmu_malloc(PTAB_INDEX_SIZE * sizeof(page_table_t), PTAB_ADDR_ALIGNMENT);
    ptab = PTAB_ADDR(ptab_addr) | PTAB_PTAB;
    early_set_page_table(ttab, TTAB_INDEX(paddr), ptab);
    for (uint32_t i = 0; i < PTAB_INDEX_SIZE; i++) {
      early_set_page_entry(ptab, i, PAGE_FAULT);
    }
  }
  page_entry_t page_entry = PAGE_ADDR(paddr) | PAGE_SHARABLE | PAGE_SMALL;
  early_set_page_entry(ptab, PTAB_INDEX(paddr), page_entry);
  return 0;
}

static int alloc_kernel_ttab(void) {
  size_t ttab_size = TTAB_INDEX_SIZE * sizeof(page_entry_t*);
  tran_table_t ttab = (tran_table_t) early_mmu_malloc(ttab_size, TTAB_ADDR_ALIGNMENT);
  for (uint32_t i = 0; i < TTAB_INDEX_SIZE; i++) {
    early_set_page_table(ttab, i, PTAB_FAULT);
  }

  for (uint32_t ptab_idx = TTAB_INDEX(0); ptab_idx < TTAB_INDEX(RAM_SIZE); ptab_idx++) {
    size_t ptab_size = PTAB_INDEX_SIZE * sizeof(page_entry_t);
    page_table_t ptab = (page_table_t) early_mmu_malloc(ptab_size, PTAB_ADDR_ALIGNMENT);
    for (uint32_t page_idx = 0; page_idx < PTAB_INDEX_SIZE; page_idx++) {
      virt_addr_t page_addr = virt_to_phys((ptab_idx << TTAB_INDEX_SHIFT) | (page_idx << PTAB_INDEX_SHIFT));
      page_entry_t page_entry = page_addr | PAGE_SMALL;
      early_set_page_entry(ptab, page_idx, page_entry);
    }
    early_set_page_table(ttab, ptab_idx, ptab | PTAB_PTAB);
  }
  kernel_ttab_paddr = (phys_addr_t) ttab;

  // We need to create a flat mapping for __start_mmu since it will turn on the
  // MMU causing the addressing to change from physical to virtual in the middle
  // of the function.
  flat_remap_paddr(ttab, 0x80300000);
  flat_remap_paddr(ttab, 0x44e09000);
  flat_remap_paddr(ttab, 0x44e09014);
  return 0;
}

static void init_mmu_registers(void) {
  __premmu_set_ttbr0((kernel_ttab_paddr & TTBR_TTAB_MASK) | TTBR_SHARABLE);
  __premmu_set_ttbcr(TTBCR_PD1 | TTBCR_USE_TTBR0);
  __premmu_set_dacr(0x00000003);
}

void __post_start_mmu(void) {
  while (true);
}

void init_mmu(void) __attribute__((noreturn));

void init_mmu(void) {
  int ret = 0;
  ret = alloc_kernel_ttab();
  if (ret < 0) {
    while (true);
  }
  init_mmu_registers();

  __start_mmu();
}
