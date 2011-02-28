#include <malloc.h>
#include <graphic/gpu.h>
#include <g-bios.h>
#include <djpeg/djpeg.h>


#ifdef CONFIG_BOOTUP_LOGO
static u16 RGB24toRGB16(u8 r, u8 g, u8 b)
{
	return (r >> 3) << 11 | (g >> 2) << 5 | b >> 3;
}
#endif


static void draw_logo(u16 * const video_buff, u32 width, u32 height, EPixFormat pix_format, void *pic_data)
{
	int i;
	u16 *vbuff;

#ifdef CONFIG_BOOTUP_LOGO
	int j, x, y;
	u8  *buff;
	u32 step_h;
	u32 step_w;
	u32 rgb_size;
	u32 line_width;
	struct djpeg_opts djpeg2bmp = *(struct djpeg_opts *)pic_data;

	step_h = (djpeg2bmp.imgbi->biHeight << 10) / height;
	step_w = (djpeg2bmp.imgbi->biWidth << 10) / width;

	rgb_size = djpeg2bmp.imgbf->bfSize - djpeg2bmp.imgbf->bfOffBits;

	buff = djpeg2bmp.rgbdata;

	vbuff = video_buff;

	line_width = rgb_size / djpeg2bmp.imgbi->biHeight;

	for (i = 0; i < height; i++)
	{
		for (j = 0; j < width * 3; j += 3)
		{
			x = (djpeg2bmp.imgbi->biHeight - ((i * step_h) >> 10) - 1) * line_width;
			y = (((j * step_w) >> 10) / 3) * 3;

			*vbuff = (u16)RGB24toRGB16(
						buff[x + y + 2],
						buff[x + y + 1],
						buff[x + y + 0]
						);
			vbuff++;
		}
	}

	free(djpeg2bmp.bmpbuf);
#else
	vbuff = video_buff;
	for (i = 0; i < width * (height / 3); i++, vbuff++)
		*vbuff = 0x001f;

	for (i = 0; i < width * (height / 3); i++, vbuff++)
		*vbuff = 0x07e0;

	for (i = 0; i < width * (height - 2 * (height / 3)); i++, vbuff++)
		*vbuff = 0xf800;
#endif
}

void *video_mem_alloc(u32 width, u32 height, EPixFormat pix_format, u32 *phy_addr)
{
	void *buff;
	u32 size = width * height;

	switch (pix_format)
	{
	case PIX_RGB15:
	case PIX_RGB16:
		size *= 2;
		break;

	case PIX_RGB24:
		size *= 3;
		break;
	}
#ifdef CONFIG_BOOTUP_LOGO
	extern unsigned char _gbios_jpg[];
	// extern unsigned int _gbios_jpg_len;

	struct djpeg_opts djpeg2bmp;

	djpeg2bmp.jpegbuf = _gbios_jpg;

	jpeg2bmp_decode(&djpeg2bmp);

	buff = dma_malloc(size, phy_addr);

	// move to the higher level ?:)
	if (buff)
	{
		draw_logo(buff, width, height, pix_format, &djpeg2bmp);
	}
#else
	buff = dma_malloc(size, phy_addr);

	// move to the higher level ?:)
	if (buff)
	{
		draw_logo(buff, width, height, pix_format, NULL);
	}
#endif

	return buff;
}
