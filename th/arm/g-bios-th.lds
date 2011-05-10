OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
	.text ALIGN(4):
	{ 
		*(.text)
	}

	.data ALIGN(4):
	{ 
		*(.data)
		. = ALIGN(4);
	}

	.rodata ALIGN(4):
	{
		*(.rodata)
		. = ALIGN(4);
	}

	.gbios_loader ALIGN(4):
	{
		g_loader_begin = .;
		*(.gbios_loader)
		g_loader_end = ALIGN(4);
	}

	.bss ALIGN(4):
	{
		bss_start = .;
		*(.bss)
		bss_end = .;
	}

	_end = ALIGN(4);
}
