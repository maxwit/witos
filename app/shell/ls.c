#include <g-bios.h>
#include <flash/flash.h>
//fixme:
#include <flash/part.h>


int main(int argc, char *argv[])
{
	char sbase[32], ssize[32];
	struct partition *part;
	struct part_attr attr;

	part = part_open(PART_CURR, OP_RDONLY);
	if (NULL == part)
	{
		printf("fail to open current parition!\n");
		return -EACCES;
	}

	part_get_attr(part, &attr);

	val_to_hr_str(attr.part_size, ssize);
	val_to_hr_str(attr.part_base, sbase);

	printf("\tPartition Type = \"%s\"\n"
		   "\tPartition Base = 0x%08x (%s)\n"
		   "\tPartition Size = 0x%08x (%s)\n"
		   "\tHost Device    = %s\n"
		   "\tMTD Deivce     = /dev/mtdblock%d\n",
		   part_type2str(attr.part_type),
		   attr.part_base, sbase,
		   attr.part_size, ssize,
		   part->host->name,
		   part_get_index(part)
		   );

	printf("\tImage File     = ");

	if (attr.image_size)
	{
		printf("\"%s\" (%d bytes)\n", attr.image_name, attr.image_size);
	}
	else
	{
		printf("(No image file)\n");
	}

	part_close(part);

	return 0;
}



