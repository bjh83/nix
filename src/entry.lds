ENTRY(_start)

SECTIONS {
  . = 0x80300000;

  .text : {
    *(.text)
    *(.rodata)
  }

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss)
    . = ALIGN(4096);
  }
  __kernel_start = 0x00301000;
}
