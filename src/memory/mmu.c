#include "boot/early_printk.h"
#include "utils/types.h"
#include "utils/base.h"

#define RAM_START 0x80000000
#define RAM_SIZE  0x20000000
#define RAM_END   (RAM_START + RAM_SIZE)

#define PAGE_SIZE                           (1 << 12)
#define PAGE_MASK                   (~(PAGE_SIZE - 1))
#define PAGE_ADDR(addr)             ((addr) & PAGE_MASK)

#define MEM_EXEC   (1 << 0)
#define MEM_IO     (1 << 1)
#define MEM_IN_USE (1 << 2)

#define PAGE_NOT_EXECUTABLE                 (1 << 0)
#define PAGE_SMALL                          (1 << 1)
#define PAGE_FAULT                                0
#define PAGE_TYPE_MASK         (((1 << 2) - 1) << 0)
#define PAGE_B                              (1 << 2)
#define PAGE_C                              (1 << 3)
#define PAGE_AP_MASK           (((1 << 2) - 1) << 4)
#define PAGE_TEX_MASK          (((1 << 3) - 1) << 6)
#define PAGE_AP_2                           (1 << 9)
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

extern phys_addr_t __stack_bottom;
static phys_addr_t next_alloc_addr = 0;

static void* early_mmu_malloc(size_t size, uint32_t alignment) {
  // early_printk("&__stack_bottom = %p\n", &__stack_bottom);
  // early_printk("__stack_bottom = %p\n", __stack_bottom);
  // early_printk("&next_alloc_addr = %p\n", &next_alloc_addr);
  if (next_alloc_addr == 0) {
    early_printk("&__stack_bottom = %p\n", &__stack_bottom);
    next_alloc_addr = (phys_addr_t) (long) &__stack_bottom;
  }

  // early_printk("next_alloc_addr = %p\n", next_alloc_addr);
  uint32_t mask = ~(alignment - 1);
  if ((next_alloc_addr & mask) != next_alloc_addr) {
    next_alloc_addr = (next_alloc_addr & mask) + alignment;
  }
  void* ret = (void *) (long) next_alloc_addr;
  next_alloc_addr += size;
  return ret;
}

typedef uint32_t page_entry_t;
typedef uint32_t virt_addr_t;

inline static virt_addr_t phys_to_virt(phys_addr_t phys) {
  return phys - RAM_START;
}

inline static phys_addr_t virt_to_phys(virt_addr_t virt) {
  return virt + RAM_START;
}

typedef long tran_table_t;
typedef long page_table_t;

phys_addr_t kernel_ttab_paddr;

inline static page_table_t get_page_table(tran_table_t ttab, uint32_t index) {
  phys_addr_t phys = (ttab & TTAB_ADDR_MASK) | ((index & TTAB_INDEX_MASK) << 2);
  virt_addr_t virt = phys_to_virt(phys);
  return *((page_table_t*) (long) virt);
}

inline static void set_page_table(tran_table_t ttab, uint32_t index, page_table_t ptab) {
  phys_addr_t phys = (ttab & TTAB_ADDR_MASK) | ((index & TTAB_INDEX_MASK) << 2);
  virt_addr_t virt = phys_to_virt(phys);
  *((page_table_t*) (long) virt) = ptab;
}

inline static page_entry_t get_page_entry(page_table_t ptab, uint32_t index) {
  phys_addr_t phys = (ptab & PTAB_ADDR_MASK) | ((index & PTAB_INDEX_MASK) << 2);
  virt_addr_t virt = phys_to_virt(phys);
  return *((page_entry_t*) (long) virt);
}

inline static void set_page_entry(page_table_t ptab, uint32_t index, page_entry_t entry) {
  phys_addr_t phys = (ptab & PTAB_ADDR_MASK) | ((index & PTAB_INDEX_MASK) << 2);
  virt_addr_t virt = phys_to_virt(phys);
  *((page_table_t*) (long) virt) = entry;
}

extern phys_addr_t __translate_addr(virt_addr_t addr);

extern void __flush_page(virt_addr_t vaddr);

extern void __start_mmu(virt_addr_t post_start_mmu_addr, virt_addr_t stack_bottom_addr) __attribute__((aligned(PAGE_SIZE), noreturn));

static phys_addr_t start_mmu_paddr;

void __post_start_mmu(void) {
  early_putchar('x');
  early_printk("MMU enabled.\n");
  early_printk("0x00000000 => %p\n", __translate_addr(0x00000000));
  early_printk("0x10000000 => %p\n", __translate_addr(0x10000000));
  early_printk("0x20000000 => %p\n", __translate_addr(0x20000000));
  early_printk("0x30000000 => %p\n", __translate_addr(0x30000000));
  early_printk("0x40000000 => %p\n", __translate_addr(0x40000000));
  early_printk("0x50000000 => %p\n", __translate_addr(0x50000000));
  early_printk("0x60000000 => %p\n", __translate_addr(0x60000000));
  early_printk("0x70000000 => %p\n", __translate_addr(0x70000000));
  early_printk("0x80000000 => %p\n", __translate_addr(0x80000000));
  early_printk("0x90000000 => %p\n", __translate_addr(0x90000000));
  early_printk("0xa0000000 => %p\n", __translate_addr(0xa0000000));
  early_printk("0xb0000000 => %p\n", __translate_addr(0xb0000000));
  early_printk("0xc0000000 => %p\n", __translate_addr(0xc0000000));
  early_printk("0xd0000000 => %p\n", __translate_addr(0xd0000000));
  early_printk("0xe0000000 => %p\n", __translate_addr(0xe0000000));
  early_printk("0xf0000000 => %p\n", __translate_addr(0xf0000000));

  // Unmap flat transition mapping.
  set_page_table(kernel_ttab_paddr, TTAB_INDEX(start_mmu_paddr), PTAB_FAULT);
  __flush_page(PAGE_ADDR(start_mmu_paddr));

  while (true);
}

static page_table_t early_get_page_table(tran_table_t ttab, uint32_t index) {
  phys_addr_t phys = (ttab & TTAB_ADDR_MASK) | ((index & TTAB_INDEX_MASK) << 2);
  return *((page_table_t*) (long) phys);
}

static void early_set_page_table(tran_table_t ttab, uint32_t index, page_table_t ptab) {
  phys_addr_t phys = (ttab & TTAB_ADDR_MASK) | ((index & TTAB_INDEX_MASK) << 2);
  *((page_table_t*) (long) (phys)) = ptab;
}

static page_entry_t early_get_page_entry(page_table_t ptab, uint32_t index) {
  phys_addr_t phys = (ptab & PTAB_ADDR_MASK) | ((index & PTAB_INDEX_MASK) << 2);
  return *((page_entry_t*) (long) phys);
}

static void early_set_page_entry(page_table_t ptab, uint32_t index, page_entry_t pentry) {
  phys_addr_t phys = (ptab & PTAB_ADDR_MASK) | ((index & PTAB_INDEX_MASK) << 2);
  *((page_entry_t*) (long) phys) = pentry;
}

static int flat_remap_paddr(tran_table_t ttab, phys_addr_t paddr) {
  page_table_t ptab = early_get_page_table(ttab, TTAB_INDEX(paddr));
  if (ptab == PTAB_FAULT) {
    phys_addr_t ptab_addr = (phys_addr_t) (long) early_mmu_malloc(PTAB_INDEX_SIZE * sizeof(page_table_t), PTAB_ADDR_ALIGNMENT);
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
  tran_table_t ttab = (tran_table_t) (long) early_mmu_malloc(ttab_size, TTAB_ADDR_ALIGNMENT);
  early_printk("Created TTBR0 at: %p\n", ttab);
  for (uint32_t i = 0; i < TTAB_INDEX_SIZE; i++) {
    early_set_page_table(ttab, i, PTAB_FAULT);
  }

  for (uint32_t ptab_idx = TTAB_INDEX(0); ptab_idx < TTAB_INDEX(RAM_SIZE); ptab_idx++) {
    size_t ptab_size = PTAB_INDEX_SIZE * sizeof(page_entry_t);
    page_table_t ptab = (page_table_t) (long) early_mmu_malloc(ptab_size, PTAB_ADDR_ALIGNMENT);
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
  start_mmu_paddr = (phys_addr_t) (long) __start_mmu;
  flat_remap_paddr(ttab, start_mmu_paddr);
  // We need to create a flat mapping for the UART ports until we set up a
  // proper driver for them.
  flat_remap_paddr(ttab, debug_uart_tx_addr);
  flat_remap_paddr(ttab, debug_uart_lsr_addr);
  return 0;
}

extern void __set_ttbr0(uint32_t new_ttbr0);
extern void __set_ttbcr(uint32_t new_ttbcr);
extern void __set_dacr(uint32_t new_dacr);

static void init_mmu_registers(void) {
  __set_ttbr0((kernel_ttab_paddr & TTBR_TTAB_MASK) | TTBR_SHARABLE);
  __set_ttbcr(TTBCR_PD1 | TTBCR_USE_TTBR0);
  __set_dacr(0x00000003);
}

void init_mmu(void) __attribute__((noreturn));

void init_mmu(void) {
  early_printk("Initializing MMU ...\n");
  int ret = 0;
  ret = alloc_kernel_ttab();
  if (ret < 0) {
    early_printk("Failed to set up kernel page tables.\n");
    while (true);
  }
  init_mmu_registers();

  virt_addr_t post_start_mmu_vaddr = phys_to_virt((phys_addr_t) (long) __post_start_mmu);
  virt_addr_t stack_bottom_addr = phys_to_virt((phys_addr_t) (long) &__stack_bottom);
  early_printk("__post_start_mmu paddr is %p\n", __post_start_mmu);
  early_printk("__post_start_mmu vaddr is %p\n", post_start_mmu_vaddr);
  __start_mmu(post_start_mmu_vaddr, stack_bottom_addr); // (phys_addr_t) (long) __post_start_mmu);
}
