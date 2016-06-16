#include "boot/early_printk.h"
#include "utils/types.h"

int main() {
  early_puts("Booting kernel ...\n");
  while (1);
  return 0;
}
