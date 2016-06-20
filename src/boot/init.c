#include "boot/early_printk.h"
#include "utils/types.h"

extern phys_addr_t __device_tree_blob;

int main() {
  early_puts("Booting kernel ...\n");
  while (1);
  return 0;
}
