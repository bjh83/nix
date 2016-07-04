#include "memory/mmu.h"
#include "utils/types.h"
#include "utils/base.h"
#include "utils/err.h"

phys_addr_t kernel_ttab_paddr;

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
