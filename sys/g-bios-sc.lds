OUTPUT_ARCH(arm)
ENTRY(_start)

SECTIONS
{
	.text ALIGN(4):
	{ 
		*(.data)
		. = ALIGN(4);
	}
}
