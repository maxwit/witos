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
	}

	.gsect_exe ALIGN(4):
	{
		g_exe_begin = .;
		*(.gsect_exe)
		g_exe_end = .;
	}

	.gsect_help ALIGN(4):
	{
		g_help_begin = .;
		*(.gsect_help)
		g_help_end = .;
	}

	.gbios_init ALIGN(4):
	{
		*(.code.init)
		*(.data.init)
		init_call_begin = .;
		*(.Level0.gbios_init)
		*(.Level1.gbios_init)
		*(.Level2.gbios_init)
		*(.Level3.gbios_init)
		*(.Level4.gbios_init)
		*(.Level5.gbios_init)
		init_call_end = .;
	}

	.bss ALIGN(4):
	{
		bss_begin = .;
		*(.bss)
		bss_end = .;
	}

	_end = ALIGN(4);
}
