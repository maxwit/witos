#include <getopt.h>
#include <graphic/display.h>

#define PIXEL_DESC(pix_fmt) \
	(pix_fmt) == PIX_RGB15 ? "RGB15" : \
		(pix_fmt) == PIX_RGB16 ? "RGB16" : \
			(pix_fmt) == PIX_RGB24 ? "RGB24" : \
				(pix_fmt) == PIX_RGB32 ? "RGB32" : "Unknown pixel format"

static void lcd_usage(void)
{
	printf("Usage: lcd <OPTION [VAL]>\n"
		"\nOPTION:\n"
		"\t-l [all]: list current video mode.\n"
		"\t-s <N>:   set current video to the N'rd mode.\n");
}

static inline int get_reflesh_rate(const struct lcd_vmode *vm)
{
	// TODO:
	return 0;
}

int main(int argc, char *argv[])
{
	int opt, ret = 0;
	extern char *optarg;
	u32 id;
	struct display *disp;
	const struct lcd_vmode *vm;

	while ((opt = getopt(argc, argv, "l::s:h")) != -1)
	{
		switch (opt)
		{
		case 'l':
			disp = get_system_display();
			if (!disp)
			{
				printf("Fail to open display!\n");
				return -ENODEV;
			}

			if (NULL == optarg)
			{
				vm = disp->video_mode;

				printf("Model: \"%s\"\n"
					"Resolution  = %d X %x (%s)\n"
					"Pixel Clock = %d (Reflesh = %d)\n"
					"HFP = %d, HBP = %d, HPW = %d\n"
					"VFP = %d, VBP = %d, VPW = %d\n",
					vm->model,
					vm->width, vm->height, PIXEL_DESC(disp->pix_fmt),
					vm->pix_clk, get_reflesh_rate(vm),
					vm->hfp, vm->hbp, vm->hpw,
					vm->vfp, vm->vbp, vm->vpw);

				break;
			}

			for (id = 0; (vm = lcd_get_vmode_by_id(id)); id++)
			{
				printf("[%d] Model: \"%s\"\n"
					"  Resolution  = %d X %x (%s)\n"
					"  Pixel Clock = %d (Reflesh = %d)\n"
					"  HFP = %d, HBP = %d, HPW = %d\n"
					"  VFP = %d, VBP = %d, VPW = %d\n\n",
					id, vm->model,
					vm->width, vm->height, PIXEL_DESC(disp->pix_fmt),
					vm->pix_clk, get_reflesh_rate(vm),
					vm->hfp, vm->hbp, vm->hpw,
					vm->vfp, vm->vbp, vm->vpw);
			}

			break;

		case 's':
			if (NULL == optarg)
			{
				lcd_usage();
				return -EINVAL;
			}

			string2value(optarg, &id);

			disp = get_system_display();
			if (!disp)
			{
				printf("Fail to open display!\n");
				return -ENODEV;
			}

			vm = lcd_get_vmode_by_id(id);

			ret = disp->set_vmode(disp, vm);
			if (ret < 0)
			{
				printf("fail to set video mode \"%s\"\n", vm->model);
			}
			break;

		default:
			ret = -EINVAL;
		case 'h':
			lcd_usage();
			return ret;
		}
	}

	return ret;
}
