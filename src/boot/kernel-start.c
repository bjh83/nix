#include "boot/early_printk.h"
#include "utils/base.h"
#include "utils/types.h"

void kernel_start(void) {
  early_printk("Starting kernel ...\n");
  while (true);
}
