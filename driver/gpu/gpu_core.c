#include <malloc.h>
#include <graphic/gpu.h>
#include "logo.h"


#ifdef CONFIG_BOOTUP_LOGO
static u16 RGB24toRGB16(u8 r, u8 g, u8 b)
{
	return (r >> 3) << 11 | (g >> 2) << 5 | b >> 3;
}
#endif


static void draw_logo(u16 * const pVideoBuffer, u32 uWidth, u32 uHeight, EPixFormat ePixFormat)
{
	int i;
	u16 *pvBuff;

#ifdef CONFIG_BOOTUP_LOGO
	extern const struct Logo gMwImage;

	pvBuff = pVideoBuffer;

	for (i = 0; i < uWidth * uHeight * 3; i += 3)
	{
		*pvBuff = RGB24toRGB16(gMwImage.pixel_data[i],
							gMwImage.pixel_data[i + 1],
							gMwImage.pixel_data[i + 2]
							);
		pvBuff++;
	}
#else
	pvBuff = pVideoBuffer;
	for (i = 0; i < uWidth * (uHeight / 3); i++, pvBuff++)
		*pvBuff = 0x001f;

	for (i = 0; i < uWidth * (uHeight / 3); i++, pvBuff++)
		*pvBuff = 0x07e0;

	for (i = 0; i < uWidth * (uHeight - 2 * (uHeight / 3)); i++, pvBuff++)
		*pvBuff = 0xf800;
#endif
}

void *video_mem_alloc(u32 uWidth, u32 uHeight, EPixFormat ePixFormat, u32 *pPhyAddr)
{
	void *pBuff;
	u32 nSize = uWidth * uHeight;

	switch (ePixFormat)
	{
	case PIX_RGB15:
	case PIX_RGB16:
		nSize *= 2;
		break;

	case PIX_RGB24:
		nSize *= 3;
		break;
	}

	pBuff = dma_malloc(nSize, pPhyAddr);

	// move to the higher level ?:)
	if (pBuff)
	{
		draw_logo(pBuff, uWidth, uHeight, ePixFormat);
	}

	return pBuff;
}
