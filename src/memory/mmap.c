#include "memory/kmalloc.h"
#include "memory/mmu.h"
#include "utils/err.h"
#include "utils/types.h"

struct page_table_slab {
  page_table_t ptab;
  struct page_table_slab* next;
};

static struct page_table_slab* page_table_slab = NULL;

static page_table_t ptab_alloc(void) {
  if (!page_table_slab) {
    void* page = page_alloc(1, MEM_DATA_NORMAL);
    for (page_table_t ptab = __pa(page); ptab < __pa(page) + PAGE_SIZE; ptab += PTAB_SIZE) {
      struct page_table_slab* new_slab = (struct page_table_slab*) kmalloc(sizeof(struct page_table_slab), MEM_DATA_NORMAL);
      new_slab->ptab = ptab;
      new_slab->next = page_table_slab;
      page_table_slab = new_slab;
    }
  }
  page_table_t ptab = page_table_slab->ptab;
  struct page_table_slab* next_slab = page_table_slab->next;
  kfree(page_table_slab);
  page_table_slab = next_slab;
  return ptab;
}

void* ioremap(phys_addr_t paddr, size_t size) {
  virt_addr_t vaddr = phys_to_virt(paddr);
  for (phys_addr_t page = PAGE_ADDR(vaddr); page < vaddr + size; page += PAGE_SIZE) {
    page_table_t ptab = get_page_table(kernel_ttab, TTAB_INDEX(page));
    if ((ptab & PTAB_TYPE_MASK) == PTAB_FAULT) {
      ptab = ptab_alloc();
      set_page_table(kernel_ttab, TTAB_INDEX(page), ptab | PTAB_PTAB);
    }
    if (set_page_attrs(page, MEM_IO) < 0) {
      return NULL;
    }
  }
  return __va(paddr);
}

