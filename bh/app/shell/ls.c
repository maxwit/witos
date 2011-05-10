#include <flash/flash.h>
//fixme:
#include <flash/part.h>

int main(int argc, char *argv[])
{
	char sbase[32], ssize[32];
	struct partition  *part;
	struct part_attr  *attr;
	struct image_info *image;

	part = part_open(PART_CURR, OP_RDONLY);
	if (NULL == part)
	{
		printf("fail to open current parition!\n");
		return -EACCES;
	}

	attr = part->attr;
	image = part->image;

	val_to_hr_str(attr->part_size, ssize);
	val_to_hr_str(attr->part_base, sbase);

	printf("\tPartition Type = \"%s\"\n"
		   "\tPartition Base = 0x%08x (%s)\n"
		   "\tPartition Size = 0x%08x (%s)\n"
		   "\tHost Device    = %s\n"
		   "\tMTD Deivce     = /dev/mtdblock%d\n",
		   part_type2str(attr->part_type),
		   attr->part_base, sbase,
		   attr->part_size, ssize,
		   part->host->name,
		   part_get_index(part)
		   );

	printf("\tImage File     = ");

	if (image->image_size)
	{
		printf("\"%s\" (%d bytes)\n", image->image_name, image->image_size);
	}
	else
	{
		printf("(No image file)\n");
	}

	part_close(part);

	return 0;
}
