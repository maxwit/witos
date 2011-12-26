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

	.gbios_font ALIGN(4):
	{
		font_list_begin = .;
		*(.gbios_font)
		font_list_end = .;
	}

	.gsect_help ALIGN(4):
	{
		g_help_begin = .;
		*(.gsect_help)
		g_help_end = .;
	}

	.gsect.board ALIGN(4):
	{
		g_board_start = .;
		*(.gsect.board)
		g_board_end = .;
	}

	.bss ALIGN(4):
	{
		bss_begin = .;
		*(.bss)
		bss_end = .;
	}

	. = ALIGN(128 << 10);
	.gsect_sysconfig ALIGN(4):
	{
		*(.gsect_sysconfig)
	}

	_end = ALIGN(4);
}
