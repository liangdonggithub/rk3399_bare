SECTIONS
{
	. = 0x00000000;
	. = ALIGN(8);
	.text :
	{
		src_loader/loader.o (.text)
		*(.text)
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
}
