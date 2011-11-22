#include <malloc.h>
#include <graphic/display.h>
#include <djpeg/djpeg.h>

static struct display* g_system_display;

#ifdef CONFIG_BOOTUP_LOGO
static __u16 RGB24toRGB16(__u8 r, __u8 g, __u8 b)
{
	return (r >> 3) << 11 | (g >> 2) << 5 | b >> 3;
}
#endif

static void draw_logo(void * const video_buff, __u32 width, __u32 height, pixel_format_t pix_format)
{
	int i;
	void *vbuff = video_buff; // fixme

#ifdef CONFIG_BOOTUP_LOGO
	int j, x, y;
	__u8  *buff;
	__u32 step_h;
	__u32 step_w;
	__u32 rgb_size;
	__u32 line_width;
	extern unsigned char _gbios_jpg[];
	// extern unsigned int _gbios_jpg_len;
	struct djpeg_opts djpeg2bmp;

	djpeg2bmp.jpegbuf = _gbios_jpg;

	jpeg2bmp_decode(&djpeg2bmp);

	step_h = (djpeg2bmp.imgbi->biHeight << 10) / height;
	step_w = (djpeg2bmp.imgbi->biWidth << 10) / width;

	rgb_size = djpeg2bmp.imgbf->bfSize - djpeg2bmp.imgbf->bfOffBits;

	buff = djpeg2bmp.rgbdata;

	line_width = rgb_size / djpeg2bmp.imgbi->biHeight;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width * 3; j += 3) {
			x = (djpeg2bmp.imgbi->biHeight - ((i * step_h) >> 10) - 1) * line_width;
			y = (((j * step_w) >> 10) / 3) * 3;

			// fixme
			switch (pix_format) {
			case PIX_RGB24:
			case PIX_RGB32:
				*((__u8 *)vbuff + 0) = buff[x + y + 0];
				*((__u8 *)vbuff + 1) = buff[x + y + 1];
				*((__u8 *)vbuff + 2) = buff[x + y + 2];

				vbuff += 4;
				break;

			case PIX_RGB15:
			case PIX_RGB16:
				*(__u16 *)vbuff = (__u16)RGB24toRGB16(
										buff[x + y + 2],
										buff[x + y + 1],
										buff[x + y + 0]
										);
				vbuff += 2;
				break;

			default:
				break;
			}
		}
	}

	free(djpeg2bmp.bmpbuf);
#else // fixme
	__u32 pix;

	struct rgb_format
	{
		int r_len, r_off;
		int g_len, g_off;
		int b_len, b_off;
		int bytes;
	} rgb;

#define MAKE_PIX(c) \
	(((1 << rgb.c##_len) - 1) << rgb.c##_off)

	switch (pix_format) {
	case PIX_RGB15:
		rgb.r_len = 5;
		rgb.r_off = 0;
		rgb.g_len = 5;
		rgb.g_off = 5;
		rgb.b_len = 5;
		rgb.b_off = 10;
		rgb.bytes = 2;
		break;

	case PIX_RGB16:
		rgb.r_len = 5;
		rgb.r_off = 0;
		rgb.g_len = 6;
		rgb.g_off = 5;
		rgb.b_len = 5;
		rgb.b_off = 11;
		rgb.bytes = 2;
		break;

	case PIX_RGB24:
	case PIX_RGB32:
		rgb.r_len = 8;
		rgb.r_off = 0;
		rgb.g_len = 8;
		rgb.g_off = 8;
		rgb.b_len = 8;
		rgb.b_off = 16;
		rgb.bytes = 4;
		break;
	}

	// fixme
	pix = MAKE_PIX(r);
	for (i = 0; i < width * (height / 3); i++) {
		memcpy(vbuff, &pix, rgb.bytes);
		vbuff += rgb.bytes;
	}

	pix = MAKE_PIX(g);
	for (i = 0; i < width * (height / 3); i++) {
		memcpy(vbuff, &pix, rgb.bytes);
		vbuff += rgb.bytes;
	}

	pix = MAKE_PIX(b);
	for (i = 0; i < width * (height / 3); i++) {
		memcpy(vbuff, &pix, rgb.bytes);
		vbuff += rgb.bytes;
	}
#endif
}

void *video_mem_alloc(__u32 *phy_addr, const struct lcd_vmode *vm, pixel_format_t pix_format)
{
	void *buff;
	__u32 size = vm->width * vm->height;

	switch (pix_format) {
	case PIX_RGB15:
	case PIX_RGB16:
		size *= 2;
		break;

	case PIX_RGB24:
	case PIX_RGB32:
		size *= 4;
		break;

	default:
		BUG();
		return NULL;
	}

	buff = dma_malloc(size, phy_addr);
	if (NULL == buff) {
		DPRINT("%s() line %d: no free memory (size = %d)!\n", __func__, __LINE__, size);
		return NULL;
	}

	// fixme: move to upper layer?
	draw_logo(buff, vm->width, vm->height, pix_format);

	return buff;
}

struct display* display_create(void)
{
	struct display *disp;

	disp = zalloc(sizeof(*disp));
	// if NULL

	return disp;
}

int display_register(struct display* disp)
{
	g_system_display = disp;
	return 0;
}

struct display* get_system_display(void)
{
	return g_system_display;
}
