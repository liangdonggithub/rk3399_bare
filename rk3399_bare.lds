OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
SECTIONS
{
	. = 0x00000000;
	. = ALIGN(8);
	__init_area_s = .;
	__init_text_s = .;
	.text :
	{
		src/start.o (.text*)
		*(.text*)
	}
	__init_text_e = .;
	. = ALIGN(8);
	__init_rodata_s = .;
	.rodata :
	{
		*(.rodata)
	}
	__init_rodata_e = .;
	. = ALIGN(8);
	__init_data_s = .;
	.data :
	{
		*(.data)
	}
	__init_data_e = .;
	. = ALIGN(8);
    __init_bss_s = .;
    .bss   ALIGN(8)  : { *(.bss)  *(COMMON) }
	. = ALIGN(8);
    __init_bss_e = .;
	. = ALIGN(8);
	__init_reladyn_s = .;
	.rela.dyn :
	{
		*(.rela*)
	}
	__init_reladyn_e = .;
	. = ALIGN(8);
	__init_stack_s = .;
	.init_stack :
	{
		. += 4096;
	}
	__init_stack_e = .;
	. = ALIGN(2048);
	__init_vector_s = .;
	.vector :
	{
		*(.vector)
	}
	__init_vector_e = .;
	__init_area_e = .;
}
