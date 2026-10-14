OUTPUT_FORMAT(elf32-i386)
ENTRY(stage_entry)
SECTIONS
{
    . = 0x10000;
    .init : { *(.init) }
    .text : { *(.text*) }
    .rodata : { *(.rodata*) }
    .data : { *(.data*) }
    . = ALIGN(16);
    __bss_start = .;
    .bss : { *(.bss*) *(COMMON) }
    . = ALIGN(16);
    __bss_end = .;
    /DISCARD/ : { *(.note*) *(.eh_frame*) *(.comment*) *(.gnu*) }
}
