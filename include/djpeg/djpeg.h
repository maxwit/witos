#pragma once

#include <g-bios.h>

#define MAKEWORD(a, b)      ((u16)(((u8)(a)) | ((u16)((u8)(b))) << 8))

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
	u16    bfType;
	u32   bfSize;
	u16    bfReserved1;
	u16    bfReserved2;
	u32   bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack()

typedef struct tagBITMAPINFOHEADER{
	u32      biSize;
	long       biWidth;
	long       biHeight;
	u16       biPlanes;
	u16       biBitCount;
	u32      biCompression;
	u32      biSizeImage;
	long       biXPelsPerMeter;
	long       biYPelsPerMeter;
	u32      biClrUsed;
	u32      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

struct djpeg_opts{
	u32 width;
	u32 height;
	u8 *jpegbuf;
	u8 *bmpbuf;
	u8 *rgbdata;
	LPBITMAPINFOHEADER imgbi;
    LPBITMAPFILEHEADER imgbf;
};

typedef struct tagRGBQUAD {
	u8    rgbBlue;
	u8    rgbGreen;
	u8    rgbRed;
	u8    rgbReserved;
} RGBQUAD;

typedef RGBQUAD  *LPRGBQUAD;

#define M_SOF0  0xc0
#define M_DHT   0xc4
#define M_EOI   0xd9
#define M_SOS   0xda
#define M_DQT   0xdb
#define M_DRI   0xdd
#define M_APP0  0xe0

#define W1 2841 /* 2048*sqrt(2)*cos(1*pi/16) */
#define W2 2676 /* 2048*sqrt(2)*cos(2*pi/16) */
#define W3 2408 /* 2048*sqrt(2)*cos(3*pi/16) */
#define W5 1609 /* 2048*sqrt(2)*cos(5*pi/16) */
#define W6 1108 /* 2048*sqrt(2)*cos(6*pi/16) */
#define W7 565  /* 2048*sqrt(2)*cos(7*pi/16) */


void init_table();
int init_tag(u8* jpegbuf);
int decode(u8* rgbbuf);
int jpeg2bmp_decode(struct djpeg_opts *djpeg2bmp);
PBITMAPINFOHEADER get_bmpinfoheader();
PBITMAPFILEHEADER get_bmpfileheader();
