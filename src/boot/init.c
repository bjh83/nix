#include "boot/early_printk.h"
#include "utils/base.h"
#include "utils/types.h"

static const phys_addr_t device_tree_blob = 0x88000000;

extern void init_mmu(void) __attribute__((noreturn));

extern notype __bss_start;
extern notype __bss_end;

static void clear_bss(void) {
  // .bss size in bytes.
  size_t bss_size = ((&__bss_end) - (&__bss_start)) * sizeof(notype);
  early_printk(".bss at: %p - %p with size %p\n", &__bss_start, &__bss_end, bss_size);
  memset(&__bss_start, 0, bss_size);
}

int main() {
  early_printk("Booting kernel ...\n");
  clear_bss();
  init_mmu();
  while (1);
  return 0;
}
