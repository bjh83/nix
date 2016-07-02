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
    __text_end = .;
  } > ram

  .data : {
    __data_start = .;
    *(.data)
    __data_end = .;
  } > ram

  .bss : {
    __bss_start = .;
    *(.bss)
  } > ram
  . = ALIGN(4);
  __bss_end = .;
  __stack_bottom = . + 0x5000;
}
