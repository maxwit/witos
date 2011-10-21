#pragma once

#define MAKEWORD(a, b)      ((__u16)(((__u8)(a)) | ((__u16)((__u8)(b))) << 8))

#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

#pragma pack(1)
typedef struct tagBITMAPFILEHEADER {
	__u16    bfType;
	__u32   bfSize;
	__u16    bfReserved1;
	__u16    bfReserved2;
	__u32   bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pack()

typedef struct tagBITMAPINFOHEADER{
	__u32      biSize;
	long       biWidth;
	long       biHeight;
	__u16       biPlanes;
	__u16       biBitCount;
	__u32      biCompression;
	__u32      biSizeImage;
	long       biXPelsPerMeter;
	long       biYPelsPerMeter;
	__u32      biClrUsed;
	__u32      biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;

struct djpeg_opts{
	__u32 width;
	__u32 height;
	__u8 *jpegbuf;
	__u8 *bmpbuf;
	__u8 *rgbdata;
	LPBITMAPINFOHEADER imgbi;
	LPBITMAPFILEHEADER imgbf;
};

typedef struct tagRGBQUAD {
	__u8    rgbBlue;
	__u8    rgbGreen;
	__u8    rgbRed;
	__u8    rgbReserved;
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
int init_tag(__u8* jpegbuf);
int decode(__u8* rgbbuf);
int jpeg2bmp_decode(struct djpeg_opts *djpeg2bmp);
PBITMAPINFOHEADER get_bmpinfoheader();
PBITMAPFILEHEADER get_bmpfileheader();
