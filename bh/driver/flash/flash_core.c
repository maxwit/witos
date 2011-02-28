#include <g-bios.h>
#include <flash/flash.h>


static struct flash_chip *g_flash_list[MAX_FLASH_DEVICES];


int flash_register(struct flash_chip *flash)
{
	int i;

	for (i = 0; i < MAX_FLASH_DEVICES; i++)
	{
		if (NULL == g_flash_list[i])
		{
			g_flash_list[i] = flash;

			return i;
		}
	}

	return -EBUSY;
}


int flash_unregister (struct flash_chip *flash)
{
	int i;

	for (i = 0; i < MAX_FLASH_DEVICES; i++)
	{
		if (flash == g_flash_list[i])
		{
			g_flash_list[i] = NULL;

			return i;
		}
	}

	return -ENODEV;
}


struct flash_chip *flash_get(unsigned int num)
{
	struct flash_chip *flash;

	if (num >= MAX_FLASH_DEVICES)
		return NULL;

	flash = g_flash_list[num];

	return flash;
}


struct flash_chip *flash_get_by_name(const char *name)
{
	int i;
	struct flash_chip *flash = NULL;

	for (i = 0; i < MAX_FLASH_DEVICES; i++)
	{
		if (g_flash_list[i] && !strcmp(name, g_flash_list[i]->name))
		{
			flash = g_flash_list[i];
			break;
		}
	}

	return flash;
}


static int __INIT__ flash_core_init(void)
{
	int i;

	for (i = 0; i < MAX_FLASH_DEVICES; i++)
		g_flash_list[i] = NULL;

	return 0;
}


SUBSYS_INIT(flash_core_init);

