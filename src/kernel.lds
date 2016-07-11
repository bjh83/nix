ENTRY(kernel_start)

SECTIONS {
  . = 0x00301000;

  .text : {
    __text_start = .;
    *(.text)
    *(.rodata)
    . = ALIGN(4096);
    __text_end = .;
  }

  .data : {
    __data_start = .;
    *(.data)
    . = ALIGN(4096);
    __data_end = .;
  }

  .bss : {
    __bss_start = .;
    *(.bss)
    . = ALIGN(4096);
  }

    . = ALIGN(4096);
  __bss_end = .;
  . = . + 0x4096;
  . = ALIGN(4096);
  __stack_bottom = .;
}
