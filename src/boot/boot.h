#ifndef BOOT_BOOT_H_
#define BOOT_BOOT_H_

#define DEBUG_UART_THR_ADDR 0x44e09000
#define DEBUG_UART_RHR_ADDR 0x44e09000
#define DEBUG_UART_LSR_ADDR 0x44e09014
#define TX_COMPLETE         1 << 5
#define RX_COMPLETE         1 << 0

#endif // BOOT_BOOT_H
