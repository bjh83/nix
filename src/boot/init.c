#include "boot/early_printk.h"
#include "utils/base.h"
#include "utils/types.h"

extern phys_addr_t __device_tree_blob;

int main() {
  char numbuf[33];
  early_printk("Booting kernel ...\n");
  early_puts(itoa(1, numbuf, 10));
  early_puts("\n");
  early_puts(itoa(12, numbuf, 10));
  early_puts("\n");
  early_puts(itoa(123, numbuf, 10));
  early_puts("\n");
  early_puts(itoa(1234, numbuf, 10));
  early_puts("\n");
  early_puts(itoa(12345, numbuf, 10));
  early_puts("\n");
  early_printk("%d, %d, %d, %d, %d ...\n", -2, -1, 0, 1, 2);
  early_printk("0x%x\n", 0xabcdef);
  early_printk("Device tree blob is at: %p", __device_tree_blob);
  while (1);
  return 0;
}
