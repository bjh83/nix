#include "memory/mmu.h"
#include "utils/types.h"
#include "utils/base.h"
#include "utils/err.h"

#include "boot/early_printk.h"

tran_table_t kernel_ttab;

#define PTAB_FULL PTAB_TYPE_MASK

tran_table_t alloc_ttab;

page_entry_t lookup_page(tran_table_t ttab, void* page_addr) {
  page_table_t ptab = get_page_table(ttab, TTAB_INDEX((virt_addr_t) page_addr));
  if ((ptab & PTAB_TYPE_MASK) == PTAB_FAULT) {
    return -EFAULT;
  }
  page_entry_t page_entry = get_page_entry(ptab, PTAB_INDEX((virt_addr_t) page_addr));
  if ((page_entry & PAGE_TYPE_MASK) == PAGE_FAULT) {
    return -EFAULT;
  }
  return page_entry;
}

int assign_page(tran_table_t ttab, void* page_addr, page_entry_t page_entry) {
  page_table_t ptab = get_page_table(ttab, TTAB_INDEX((virt_addr_t) page_addr));
  if ((ptab & PTAB_TYPE_MASK) == PTAB_FAULT) {
    return -EFAULT;
  }
  set_page_entry(ptab, PTAB_INDEX((virt_addr_t) page_addr), page_entry);
  return 0;
}

static inline long indices_to_addr(uint32_t ptab_idx, uint32_t page_idx) {
  return (ptab_idx << TTAB_INDEX_SHIFT) | (page_idx << PTAB_INDEX_SHIFT);
}

void flush_mem(void* mem, size_t size) {
  virt_addr_t end_addr = ((virt_addr_t) mem) + size;
  for (virt_addr_t page_addr = PAGE_ADDR((virt_addr_t) mem); page_addr < end_addr; page_addr += PAGE_SIZE) {
    __flush_page(page_addr);
  }
}

static phys_addr_t next_alloc_addr = 0;

static phys_addr_t early_mmu_malloc(size_t size, uint32_t alignment) {
  uint32_t mask = ~(alignment - 1);
  if ((next_alloc_addr & mask) != next_alloc_addr) {
    next_alloc_addr = (next_alloc_addr & mask) + alignment;
  }
  phys_addr_t ret = next_alloc_addr;
  next_alloc_addr += size;
  return ret;
}

static int inc_usage(phys_addr_t page_addr) {
  page_table_t ptab = get_page_table(alloc_ttab, TTAB_INDEX(page_addr));
  if ((ptab & PTAB_TYPE_MASK) == PTAB_FAULT) {
    return -EFAULT;
  }
  int32_t share_count = get_page_entry(ptab, PTAB_INDEX(page_addr)) + 1;
  set_page_entry(ptab, PTAB_INDEX(page_addr), share_count);
  return share_count;
}

static int dec_usage(phys_addr_t page_addr) {
  page_table_t ptab = get_page_table(alloc_ttab, TTAB_INDEX(page_addr));
  if ((ptab & PTAB_TYPE_MASK) == PTAB_FAULT) {
    return -EFAULT;
  }
  int32_t share_count = get_page_entry(ptab, PTAB_INDEX(page_addr)) - 1;
  set_page_entry(ptab, PTAB_INDEX(page_addr), share_count);
  return share_count;
}

static int get_usage(phys_addr_t page_addr) {
  page_table_t ptab = get_page_table(alloc_ttab, TTAB_INDEX(page_addr));
  if ((ptab & PTAB_TYPE_MASK) == PTAB_FAULT) {
    return -EFAULT;
  }
  int32_t share_count = get_page_entry(ptab, PTAB_INDEX(page_addr));
  return share_count;
}

static bool is_ptab_free(phys_addr_t ptab_addr) {
  page_table_t ptab = get_page_table(alloc_ttab, TTAB_INDEX(ptab_addr));
  return (ptab & PTAB_TYPE_MASK) != PTAB_FULL;
}

int set_page_attrs(virt_addr_t page_addr, uint32_t attrs) {
  page_entry_t page_entry = lookup_page(kernel_ttab, (void*) page_addr);
  if (IS_ERR(page_entry)) {
    return (int) page_entry;
  }

  page_entry = PAGE_ADDR(page_addr) | PAGE_SMALL;
  if (!(attrs & MEM_EXEC)) {
    page_entry |= PAGE_NOT_EXECUTABLE;
  }
  if (!(attrs & MEM_WRITE)) {
    page_entry |= PAGE_AP_RO_PL1;
  }
  if (attrs & MEM_CACHE) {
    page_entry |= PAGE_TEXCB_WRITE_NO_ALLOC;
  }

  return assign_page(kernel_ttab, (void*) page_addr, page_entry);
}

static int mmu_reserve_block(void* start_ptr, void* end_ptr, uint32_t attrs) {
  virt_addr_t vaddr_start = (virt_addr_t) start_ptr;
  virt_addr_t vaddr_end = (virt_addr_t) end_ptr;
  phys_addr_t paddr_start = virt_to_phys(vaddr_start);
  phys_addr_t paddr_end = virt_to_phys(vaddr_end);
  for (phys_addr_t page_addr = PAGE_ADDR(paddr_start); page_addr < paddr_end; page_addr += PAGE_SIZE) {
    int share_count = inc_usage(page_addr);
    if (share_count < 0) {
      return share_count;
    }
  }
  for (virt_addr_t page_addr = PAGE_ADDR(vaddr_start); page_addr < vaddr_end; page_addr += PAGE_SIZE) {
    int ret = set_page_attrs(page_addr, attrs);
    if (ret < 0) {
      return ret;
    }
  }
  return 0;
}

static inline bool is_ptab_boundary(phys_addr_t paddr) {
  return paddr == (TTAB_INDEX(paddr) << TTAB_INDEX_SHIFT);
}

void* page_alloc(size_t page_num, uint32_t attrs) {
  phys_addr_t start_addr = INVALID_PADDR;
  for (phys_addr_t page_addr = RAM_START; page_addr < RAM_START + RAM_SIZE; page_addr += 0) {
    if (is_ptab_boundary(page_addr) && !is_ptab_free(page_addr)) {
      start_addr = INVALID_PADDR;
      page_addr += (1 << TTAB_INDEX_SHIFT);
      continue;
    }

    if (get_usage(page_addr) > 0) {
      start_addr = INVALID_PADDR;
      continue;
    }

    if (start_addr == INVALID_PADDR) {
      start_addr = page_addr;
    }

    if (((page_addr - start_addr) >> PAGE_SHIFT) + 1 >= page_num) {
      void* block_ptr = __va(start_addr);
      int ret = mmu_reserve_block(block_ptr, __va(page_addr), attrs);
      if (ret < 0) {
        return ERR_PTR(ret);
      }
      return block_ptr;
    }
  }
  return ERR_PTR(-ENOMEM);
}

static int __page_free(void* page) {
  int result = assign_page(kernel_ttab, page, PAGE_FAULT);
  if (result < 0) {
    return result;
  }
  phys_addr_t page_paddr = __pa(page);
  int share_count = dec_usage(page_paddr);
  if (share_count == 0) {
    page_table_t ptab = get_page_table(alloc_ttab, TTAB_INDEX(page_paddr));
    set_page_table(alloc_ttab, TTAB_INDEX(page_paddr), ptab | PTAB_PTAB);
  }
  return 0;
}

void page_free(void* page, size_t page_num) {
    page = (void*) PAGE_ADDR((long) page);
  for (size_t i = 0; i < page_num; i++) {
    __page_free(page);
    page = (void*) (((long) page) + PAGE_SIZE);
  }
}

static int mmu_reap_unused(void) {
  for (uint32_t ptab_idx = 0; ptab_idx < TTAB_INDEX(RAM_SIZE); ptab_idx++) {
    int pages_used = 0;
    for (uint32_t page_idx = 0; page_idx < PTAB_INDEX_SIZE; page_idx++) {
      virt_addr_t page_vaddr = (ptab_idx << TTAB_INDEX_SHIFT) | (page_idx << PTAB_INDEX_SHIFT);
      phys_addr_t page_paddr = virt_to_phys(page_vaddr);
      if (get_usage(page_paddr) > 0) {
        pages_used++;
      } else {
        assign_page(kernel_ttab, (void*) page_vaddr, PAGE_FAULT);
      }
    }
    if (pages_used == PTAB_INDEX_SIZE) {
      page_table_t ptab = get_page_table(alloc_ttab, ptab_idx);
      set_page_table(alloc_ttab, ptab_idx, ptab | PTAB_FULL);
    }
  }
  return 0;
}

extern notype __text_start;
extern notype __text_end;
extern notype __data_start;
extern notype __data_end;
extern notype __bss_start;
extern notype __bss_end;

int init_mmu(void) {
  early_printk("init_mmu ...\n");
  __disable_irqs();
  early_printk("0x44e09000 => %p\n", __translate_addr(0x44e09000));
  for (virt_addr_t page = 0; page < RAM_START + RAM_SIZE; page += PAGE_SIZE) {
    if ((page >> 20) << 20 == page) {
      early_printk("flushing page %p\n", page);
    }
    if (!(0x44e00000 <= page && page <= 0x44f00000) || true) {
      __flush_page(page);
    }
  }
  {
    kernel_ttab = __get_ttbr0();
    phys_addr_t last_ram_paddr = RAM_SIZE - 1;
    page_table_t ptab = get_page_table(kernel_ttab, TTAB_INDEX(last_ram_paddr));
    phys_addr_t last_used_addr = (ptab & PTAB_ADDR_MASK);
    next_alloc_addr = last_used_addr + PTAB_SIZE;
    early_printk("next_alloc_addr = %p\n", next_alloc_addr);
  }

  early_printk("creating alloc_ttab ...\n");

  alloc_ttab = early_mmu_malloc(sizeof(page_table_t) * TTAB_INDEX_SIZE, TTAB_ADDR_ALIGNMENT) & TTAB_ADDR_MASK;
  for (uint32_t i = 0; i < TTAB_INDEX_SIZE; i++) {
    set_page_table(alloc_ttab, i, PTAB_FAULT);
  }

  for (uint32_t ptab_idx = TTAB_INDEX(RAM_START); ptab_idx < TTAB_INDEX(RAM_START + RAM_SIZE); ptab_idx++) {
    early_printk("creating ptab at %d\n", ptab_idx);
    page_table_t ptab = early_mmu_malloc(sizeof(page_entry_t) * PTAB_INDEX_SIZE, PTAB_ADDR_ALIGNMENT) & PTAB_ADDR_MASK;
    __flush_page(phys_to_virt(ptab));
    early_printk("populating ptab at %d, ptab = %p\n", ptab_idx, ptab);
    for (uint32_t page_idx = 0; page_idx < PTAB_INDEX_SIZE; page_idx++) {
      if (ptab_idx == 2080) {
        early_printk("populating page entry at %d\n", page_idx);
      }
      set_page_entry(ptab, page_idx, 0);
    }
    early_printk("populating ptab at %d complete\n", ptab_idx);
    set_page_table(alloc_ttab, ptab_idx, ptab | PTAB_PTAB);
    early_printk("populating alloc_ttab at %d\n", ptab_idx);
  }
  early_printk("alloced alloc_ttab ...\n");

  {
    phys_addr_t kernel_ttab_paddr = kernel_ttab & TTAB_ADDR_MASK;
    mmu_reserve_block(__va(kernel_ttab_paddr), __va(kernel_ttab_paddr + TTAB_INDEX_SIZE + 1), MEM_DATA_NORMAL);
    for (uint32_t i = 0; i < TTAB_INDEX_SIZE; i++) {
      page_table_t ptab = get_page_table(kernel_ttab, i);
      if (ptab != PAGE_FAULT) {
        phys_addr_t ptab_paddr = ptab & PTAB_ADDR_MASK;
        mmu_reserve_block(__va(ptab_paddr), __va(ptab_paddr + PTAB_INDEX_SIZE), MEM_DATA_NORMAL);
      }
    }
  }
  early_printk("reserved kernel_ttab memory\n");
  {
    phys_addr_t alloc_ttab_paddr = alloc_ttab & TTAB_ADDR_MASK;
    mmu_reserve_block(__va(alloc_ttab_paddr), __va(alloc_ttab_paddr + TTAB_INDEX_SIZE + 1), MEM_DATA_NORMAL);
    for (uint32_t i = 0; i < TTAB_INDEX_SIZE; i++) {
      page_table_t ptab = get_page_table(alloc_ttab, i);
      if (ptab != PAGE_FAULT) {
        phys_addr_t ptab_paddr = ptab & PTAB_ADDR_MASK;
        mmu_reserve_block(__va(ptab_paddr), __va(ptab_paddr + PTAB_INDEX_SIZE), MEM_DATA_NORMAL);
      }
    }
  }

  early_printk("About to reserve memory.\n");

  mmu_reserve_block((void*) (((virt_addr_t) &__text_start) - STACK_SIZE), &__text_start, MEM_DATA_NORMAL);
  mmu_reserve_block(&__text_start, &__text_end, MEM_EXEC_NORMAL);
  mmu_reserve_block(&__data_start, &__data_end, MEM_DATA_NORMAL);
  mmu_reserve_block(&__bss_start, &__bss_end, MEM_DATA_NORMAL);

  early_printk("About to reap unused memory.\n");

  mmu_reap_unused();

  return 0;
}
