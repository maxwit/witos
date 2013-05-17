#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <graphic/display.h>

#define PIXEL_DESC(pix_fmt) \
	(pix_fmt) == PIX_RGB15 ? "RGB15" : \
		(pix_fmt) == PIX_RGB16 ? "RGB16" : \
			(pix_fmt) == PIX_RGB24 ? "RGB24" : \
				(pix_fmt) == PIX_RGB32 ? "RGB32" : "Unknown pixel format"

static inline int get_reflesh_rate(const struct lcd_vmode *vm)
{
	// TODO:
	return 0;
}

static inline int vmode_print(const struct lcd_vmode *vm, const struct display *disp)
{
	if (vm == NULL || disp == NULL) {
		return -1;
	}

	printf("Model: \"%s\"\n"
		"Resolution  = %d X %d (%s)\n"
		"Pixel Clock = %d (Reflesh = %d)\n"
		"HFP = %d, HBP = %d, HPW = %d\n"
		"VFP = %d, VBP = %d, VPW = %d\n",
		vm->model,
		vm->width, vm->height, PIXEL_DESC(disp->pix_fmt),
		vm->pix_clk, get_reflesh_rate(vm),
		vm->hfp, vm->hbp, vm->hpw,
		vm->vfp, vm->vbp, vm->vpw);

	return 0;
}

int main(int argc, char *argv[])
{
	int opt, ret = 0;
	extern char *optarg;
	__u32 id;
	struct display *disp;
	const struct lcd_vmode *vm;

	if (argc == 1) {
		usage();
		return -EINVAL;
	}

	while ((opt = getopt(argc, argv, "l::s:h")) != -1) {
		switch (opt) {
		case 'l':
			disp = get_system_display();
			if (!disp) {
				printf("Fail to open display!\n");
				return -ENODEV;
			}

			if (NULL == optarg) {
				vm = disp->video_mode;
				vmode_print(vm, disp);

				break;
			}

			if (strcmp("all", optarg) != 0) {
				usage();
				return -EINVAL;
			}

			for (id = 0; (vm = lcd_get_vmode_by_id(id)); id++) {
				printf("\n[%d] ", id);
				vmode_print(vm, disp);
			}

			break;

		case 's':
			if (NULL == optarg) {
				usage();
				return -EINVAL;
			}

			if (str_to_val(optarg, (unsigned long *)&id) < 0) {
				vm = lcd_get_vmode_by_name(optarg);
			} else {
				vm = lcd_get_vmode_by_id(id);
			}
			if (vm == NULL) {
				printf("Fail to get display mode!\n");
				return -EINVAL;
			}

			disp = get_system_display();
			if (!disp) {
				printf("Fail to open display!\n");
				return -ENODEV;
			}

			ret = disp->set_vmode(disp, vm);
			if (ret < 0) {
				printf("Fail to set video mode \"%s\"!\n", vm->model);
			} else {
				printf("Success to set video mode \"%s\"!\n", vm->model);
			}
			break;

		default:
			ret = -EINVAL;
		case 'h':
			usage();
			return ret;
		}
	}

	return ret;
}
