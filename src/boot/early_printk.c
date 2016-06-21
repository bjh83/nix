#include "utils/types.h"

static const phys_addr_t debug_uart_tx_addr  = 0x44e09000;
static const phys_addr_t debug_uart_rx_addr  = 0x44e09000;
static const phys_addr_t debug_uart_lsr_addr = 0x44e09014;
static const int tx_complete = 1 << 5;
static const int rx_complete = 1 << 0;

static bool is_tx_complete(void) {
  volatile uint32_t* debug_uart_lsr = (volatile uint32_t*) debug_uart_lsr_addr;

  return (*debug_uart_lsr & tx_complete) > 0;
}

int early_putchar(char c) {
  volatile uint32_t* debug_uart_tx = (volatile uint32_t*) debug_uart_tx_addr;

  while (!is_tx_complete());
  *debug_uart_tx = c;
  if (c == '\n') {
    while (!is_tx_complete());
    *debug_uart_tx = '\r';
  }
  return 0;
}

void early_puts(const char* str) {
  for (int i = 0; str[i] != '\0'; i++) {
    if (str[i] != '\n') {
      early_putchar(str[i]);
    } else {
      early_putchar('\n');
      early_putchar('\r');
    }
  }
}
