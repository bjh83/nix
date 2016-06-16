#include "boot/boot.h"
#include "types.h"

bool tx_complete() {
  volatile uint32_t* debug_uart_lsr = (volatile uint32_t*) DEBUG_UART_LSR_ADDR;

  return (*debug_uart_lsr & TX_COMPLETE) > 0;
}

void early_putchar(char c) {
  volatile uint32_t* debug_uart_thr = (volatile uint32_t*) DEBUG_UART_THR_ADDR;

  // Wait for TX to become available.
  while (!tx_complete());
  *debug_uart_thr = c;
}

void early_puts(const char* str) {
  for (int i = 0; str[i] != '\0'; i++) {
    early_putchar(str[i]);
  }
}

int main() {
  early_puts("Hello, world!\n");
  while (1);
  return 0;
}
