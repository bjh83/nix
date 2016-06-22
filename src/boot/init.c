#include "boot/early_printk.h"
#include "utils/base.h"
#include "utils/types.h"

static const phys_addr_t device_tree_blob = 0x88000000;

extern int parse_device_tree(phys_addr_t device_tree_blob);

int main() {
  early_printk("Booting kernel ...\n");
  early_printk("Device tree blob is at: %p\n", device_tree_blob);
  parse_device_tree(device_tree_blob);
  while (1);
  return 0;
}
