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
#define PTAB                          (1 << 0)
#define PTAB_NOT_EXECUTABLE           (1 << 2)
#define PTAB_NOT_SECURE               (1 << 3)
#define PTAB_DOMAIN_MASK (((1 << 4) - 1) << 5)
#define PTAB_IMPL_DEFN                (1 << 9)
#define PTAB_SIZE                     (1 << 10)
#define PTAB_MASK       (~(PAGE_TABLE_SIZE - 1))
#define PTAB_ADDR(addr) ((addr) & PAGE_TABLE_MASK)

#define TTAB_INDEX_SIZE     (1 << 12)
#define TTAB_INDEX_SHIFT          20
#define TTAB_INDEX(addr) (((addr) & ((TTAB_INDEX_SIZE - 1) << TTAB_INDEX_SHIFT)) >> TTAB_INDEX_SHIFT)
#define TTAB_ADDR_ALIGNMENT (1 << 14)
#define TTAB_ADDR_MASK    (~(TTAB_ADDR_ALIGNMENT - 1))
#define PTAB_INDEX_SIZE     (1 << 8)
#define PTAB_INDEX_SHIFT          12
#define PTAB_INDEX(addr) (((addr) & ((PTAB_INDEX_SIZE - 1) << PTAB_INDEX_SHIFT)) >> PTAB_INDEX_SHIFT)
#define PTAB_ADDR_ALIGNMENT (1 << 10)
#define PTAB_ADDR_MASK    (~(PTAB_ADDR_ALIGNMENT - 1))
#define PAGE_INDEX(addr)  ((addr) & (PAGE_SIZE - 1))

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

static virt_addr_t phys_to_virt(phys_addr_t phys) {
  return phys - RAM_START;
}

static phys_addr_t virt_to_phys(virt_addr_t virt) {
  return virt + RAM_START;
}

extern void __start_mmu(void) __attribute__((aligned(PAGE_SIZE), noreturn));

void __post_start_mmu(void) {
  early_printk("MMU enabled.\n");
  while (true);
}

static int flat_remap_paddr(page_entry_t** kernel_ttab, const void* ptr) {
  phys_addr_t paddr = (phys_addr_t) (long) ptr;
  page_entry_t* ptab = kernel_ttab[TTAB_INDEX(paddr)];
  if (!ptab) {
    // Create page table.
    kernel_ttab[TTAB_INDEX(paddr)] = (page_entry_t*) early_mmu_malloc(PTAB_INDEX_SIZE, PTAB_ADDR_ALIGNMENT);
    ptab = kernel_ttab[TTAB_INDEX(paddr)];
    // Mark all pages as faults.
    memset(ptab, PAGE_FAULT, PTAB_INDEX_SIZE);
  }
  // Add mapping for ptr.
  ptab[PTAB_INDEX(paddr)] = PAGE_ADDR(paddr) | PAGE_SMALL;
  return 0;
}

phys_addr_t kernel_ttab_paddr;

static int alloc_kernel_ttab(void) {
  size_t kernel_ttab_size = TTAB_INDEX_SIZE * sizeof(page_entry_t*);
  page_entry_t** kernel_ttab = (page_entry_t**) early_mmu_malloc(kernel_ttab_size, TTAB_ADDR_ALIGNMENT);
  early_printk("Created TTBR0 at: %p\n", kernel_ttab);
  memset(kernel_ttab, 0, kernel_ttab_size);
  for (size_t ttab_idx = TTAB_INDEX(0); ttab_idx < TTAB_INDEX(RAM_SIZE); ttab_idx++) {
    size_t ptab_size = PTAB_INDEX_SIZE * sizeof(page_entry_t);
    page_entry_t* ptab = (page_entry_t*) early_mmu_malloc(ptab_size, PTAB_ADDR_ALIGNMENT);
    for (size_t ptab_idx = 0; ptab_idx < PTAB_INDEX_SIZE; ptab_idx++) {
      virt_addr_t page_addr = (ttab_idx << TTAB_INDEX_SHIFT) | (ptab_idx << PTAB_INDEX_SHIFT);
      page_entry_t page_entry = page_addr | PAGE_SMALL;
      ptab[ptab_idx] = page_entry;
    }
    kernel_ttab[ttab_idx] = ptab;
  }
  kernel_ttab_paddr = (phys_addr_t) (long) kernel_ttab;

  // We need to create a flat mapping for __start_mmu since it will turn on the
  // MMU causing the addressing to change from physical to virtual in the middle
  // of the function.
  // flat_remap_paddr(kernel_ttab, (void*) __start_mmu);
  // We need to create a flat mapping for the UART ports until we set up a
  // proper driver for them.
  flat_remap_paddr(kernel_ttab, (void*) (long) debug_uart_tx_addr);
  flat_remap_paddr(kernel_ttab, (void*) (long) debug_uart_lsr_addr);
  return 0;
}

extern void __set_ttbr0(uint32_t new_ttbr0);
extern void __set_ttbcr(uint32_t new_ttbcr);

static void init_mmu_registers(void) {
  __set_ttbr0((kernel_ttab_paddr & TTBR_TTAB_MASK) | TTBR_SHARABLE);
  __set_ttbcr(TTBCR_PD0 | TTBCR_USE_TTBR0);
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

  __start_mmu();
}
