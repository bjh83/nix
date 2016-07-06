ENTRY(_start)

MEMORY {
  ram : ORIGIN = 0x80300000, LENGTH = 0x10000
}

SECTIONS {
  .text : {
    __text_start = .;
    boot/entry.o(.text)
    *(.text)
    *(.rodata)
    . = ALIGN(4096);
    __text_end = .;
  } > ram

  .data : {
    __data_start = .;
    *(.data)
    . = ALIGN(4096);
    __data_end = .;
  } > ram

  .bss : {
    __bss_start = .;
    *(.bss)
    . = ALIGN(4096);
  } > ram
    . = ALIGN(4096);
  __bss_end = .;
  . = . + 0x4096;
  . = ALIGN(4096);
  __stack_bottom = .;
}
