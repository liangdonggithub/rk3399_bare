SECTIONS
{
	. = 0x00000000;
	. = ALIGN(8);
	.text :
	{
		src/start.o (.text*)
		src/gic.o (.text*)
	}
	. = ALIGN(8);
	.rodata :
	{
		*(.rodata)
	}
	. = ALIGN(8);
	.data :
	{
		*(.data)
	}
	. = ALIGN(8);
    __bss_start = .;
    .bss   ALIGN(8)  : { *(.bss)  *(COMMON) }
	. = ALIGN(8);
    __bss_end = .;
}
