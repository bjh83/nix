#include "memory/mmu.h"
#include "utils/err.h"
#include "utils/types.h"

#define MEM_EXEC        (1 << 0)
#define MEM_WRITE       (1 << 1)
#define MEM_CACHE       (1 << 2)
#define MEM_IO          (1 << 3)

#define MEM_EXEC_NORMAL (MEM_CACHE | MEM_EXEC)
#define MEM_DATA_NORMAL (MEM_CACHE | MEM_WRITE)

static int set_page_attrs(virt_addr_t page_addr, uint32_t attrs) {
  page_entry_t page_entry = lookup_page(kernel_ttab_paddr, (void*) page_addr);
  if (IS_ERR(page_entry)) {
    return (int) page_entry;
  }

  page_entry = 0;
  if (!(attrs & MEM_EXEC)) {
    page_entry |= PAGE_NOT_EXECUTABLE;
  }
  if (!(attrs & MEM_WRITE)) {
    page_entry |= PAGE_AP_RO_PL1;
  }
  if (attrs & MEM_CACHE) {
    page_entry |= PAGE_TEXCB_WRITE_NO_ALLOC;
  }

  return assign_page(kernel_ttab_paddr, (void*) page_addr, page_entry);
}

void* set_mem_attrs(void* mem, size_t size, uint32_t attrs) {
  virt_addr_t end_addr = ((virt_addr_t) mem) + size;
  for (virt_addr_t page_addr = PAGE_ADDR((virt_addr_t) mem); page_addr < end_addr; page_addr += PAGE_SIZE) {
    int result = set_page_attrs(page_addr, attrs);
    flush_page(page_addr);
    if (result < 0) {
      return ERR_PTR(result);
    }
  }
  return mem;
}

