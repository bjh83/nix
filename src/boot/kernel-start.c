#include "boot/early_printk.h"
#include "utils/base.h"
#include "utils/types.h"

extern notype __bss_start;
extern notype __bss_end;
extern int init_mmu(void);

void init_bss(void) {
  void* bss_start = &__bss_start;
  void* bss_end = &__bss_end;
  size_t bss_size = ((size_t) (long) bss_end) - ((size_t) (long) bss_start);
  memset(bss_start, 0, bss_size);
}

void kernel_start(void) {
  early_printk("Starting kernel ...\n");
  init_bss();
  init_mmu();
  while (true);
}
