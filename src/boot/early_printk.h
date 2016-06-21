#ifndef BOOT_EARLY_PRINTK_H_
#define BOOT_EARLY_PRINTK_H_

#include "utils/base.h"

extern int early_putchar(char c);

#define early_puts(fmt) putchar_to_puts(early_putchar, fmt)

#define early_printk(fmt, ...) putchar_to_printf(early_putchar, fmt, ##__VA_ARGS__)

#endif // BOOT_EARLY_PRINTK_H_
