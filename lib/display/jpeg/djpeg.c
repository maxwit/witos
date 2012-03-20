#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <djpeg/djpeg.h>

#define WIDTHBYTES(i)    ((i + 31) / 32 * 4)

static BITMAPFILEHEADER   bf;
static BITMAPINFOHEADER   bi;
static __u32     line_ubytes;
static __u32     img_w, img_h;
static char*   lpPtr;

static short   SampRate_Y_H, SampRate_Y_V;
static short   SampRate_U_H, SampRate_U_V;
static short   SampRate_V_H, SampRate_V_V;
static short   H_YtoU, V_YtoU, H_YtoV, V_YtoV;
static short   Y_in_MCU, U_in_MCU, V_in_MCU;
static __u8      *lp;
static short   qt_table[3][64];
static short   comp_num;
static __u8      comp_index[3];
static __u8      YDcIndex, YAcIndex, UVDcIndex, UVAcIndex;
static __u8      HufTabIndex;
static short   *YQtTable, *UQtTable, *VQtTable;
static __u8      And[9] = {0, 1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};
static short   code_pos_table[4][16],code_len_table[4][16];
static __u16     code_value_table[4][256];
static __u16     huf_max_value[4][16],huf_min_value[4][16];
static short   bit_pos, cur_byte;
static short   rrun, vvalue;
static short   MCUBuffer[10 * 64];
static int     QtZzMCUBuffer[10 * 64];
static short   BlockBuffer[64];
static short   ycoef, ucoef, vcoef;
static bool    interval_flag;
static short   interval = 0;
static int     Y[4 * 64], U[4 * 64], V[4 * 64];
static __u32     size_i, size_j;
static short   restart;
static long    iclip[1024];
static long    *iclp;
// static __u32     biBitCount = 24;
static int Zig_Zag[8][8] = {
	{0,1,5,6,14,15,27,28},
	{2,4,7,13,16,26,29,42},
	{3,8,12,17,25,30,41,43},
	{9,11,18,24,37,40,44,53},
	{10,19,23,32,39,45,52,54},
	{20,22,33,38,46,51,55,60},
	{21,34,37,47,50,56,59,61},
	{35,36,48,49,57,58,62,63}
};

void init_table()
{
	size_i = size_j = 0;
	img_w = img_h = 0;
	rrun=vvalue = 0;
	bit_pos = 0;
	cur_byte = 0;
	interval_flag = false;
	restart = 0;
	comp_num = 0;
	HufTabIndex = 0;
	ycoef = ucoef = vcoef = 0;

	memset(qt_table, 0, 3 * 64 * sizeof(short));
	memset(code_len_table, 0, 4 * 64 * sizeof(short));
	memset(code_pos_table, 0, 4 * 64 * sizeof(short));
	memset(huf_max_value,  0, 4 * 64 * sizeof(__u16));
	memset(huf_min_value,  0, 4 * 64 * sizeof(__u16));
	memset(code_value_table,  0, 4 * 256 * sizeof(__u16));
	memset(MCUBuffer, 0, 10 * 64 * sizeof(short));
	memset(QtZzMCUBuffer, 0, 10 * 64 * sizeof(int));
	memset(BlockBuffer, 0, 64 * sizeof(short));
	memset(Y, 0, 64 * sizeof(int));
	memset(U, 0, 64 * sizeof(int));
	memset(V, 0, 64 * sizeof(int));
	memset(comp_index, 0, 3 * sizeof(__u8));
}

PBITMAPINFOHEADER get_bmpinfoheader()
{
	return &bi;
}

PBITMAPFILEHEADER get_bmpfileheader()
{
	return &bf;
}

void creat_bmphead()
{
	__u32 num_colors, ImgSize;
	memset((char *)&bf, 0, sizeof(BITMAPFILEHEADER));
	memset((char *)&bi, 0, sizeof(BITMAPINFOHEADER));

	bi.biSize = (__u32)sizeof(BITMAPINFOHEADER);
	bi.biWidth = (long)(img_w);
	bi.biHeight = (long)(img_h);
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;
	bi.biCompression = BI_RGB;
	num_colors = 0;
	line_ubytes = (__u32)WIDTHBYTES(bi.biWidth * bi.biBitCount);
	printf("line_ubytes = %d\n", line_ubytes);
	ImgSize = (__u32)line_ubytes * bi.biHeight;

	bf.bfType = 0x4d42;
	bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + num_colors * sizeof(RGBQUAD) + ImgSize;
	bf.bfOffBits = (__u32)(num_colors * sizeof(RGBQUAD) + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
	//BufSize = bf.bfSize - sizeof(BITMAPFILEHEADER);
}

int init_tag(__u8 *jpegbuf)
{
	bool finish = false;
	__u8 id;
	short  llength;
	short  i, j, k;
	short  huftab1, huftab2;
	short  huftabindex;
	__u8 hf_table_index;
	__u8 qt_table_index;
	__u8 comnum;
	__u8 *lptemp;
	short  ccount;

	lp = jpegbuf + 2;
	while (!finish) {
		id = *(lp + 1);
		lp += 2;
		switch (id) {
		case M_APP0:
			llength = MAKEWORD(*(lp + 1), *lp);
			lp += llength;
			break;

		case M_DQT:
			llength = MAKEWORD(*(lp + 1), *lp);
			qt_table_index = (*(lp + 2)) & 0x0f;
			lptemp = lp + 3;
			if (llength < 80) {
				for (i = 0; i < 64; i++)
					qt_table[qt_table_index][i] = (short)*(lptemp++);
			} else {
				for ( i = 0; i < 64; i++)
					qt_table[qt_table_index][i] = (short)*(lptemp++);

	            qt_table_index = (*(lptemp++)) & 0x0f;
				for(i = 0; i < 64; i++)
					qt_table[qt_table_index][i] = (short)*(lptemp++);
			}

			lp += llength;
			break;

		case M_SOF0:
			llength = MAKEWORD(*(lp + 1), *lp);
			img_h = MAKEWORD(*(lp + 4), *(lp + 3));
			img_w = MAKEWORD(*(lp + 6), *(lp + 5));
	        comp_num = *(lp + 7);
			if((comp_num != 1)&&(comp_num != 3))
				return -1;

			if(comp_num == 3) {
				comp_index[0] = *(lp + 8);
				SampRate_Y_H = (*(lp + 9)) >> 4;
				SampRate_Y_V = (*(lp + 9)) & 0x0f;
				YQtTable = (short *)qt_table[*(lp + 10)];
				comp_index[1] = *(lp + 11);
				SampRate_U_H = (*(lp + 12)) >> 4;
				SampRate_U_V = (*(lp + 12)) & 0x0f;
				UQtTable = (short *)qt_table[*(lp + 13)];
				comp_index[2] = *(lp + 14);
				SampRate_V_H = (*(lp + 15)) >>4;
				SampRate_V_V = (*(lp + 15)) & 0x0f;
				VQtTable = (short *)qt_table[*(lp + 16)];
			} else {
				comp_index[0] = *(lp + 8);
				SampRate_Y_H = (*(lp + 9)) >>4;
				SampRate_Y_V = (*(lp+9)) & 0x0f;
				YQtTable = (short *)qt_table[*(lp + 10)];
				comp_index[1] = *(lp + 8);
				SampRate_U_H = 1;
				SampRate_U_V = 1;
				UQtTable = (short *)qt_table[*(lp + 10)];
				comp_index[2] = *(lp + 8);
				SampRate_V_H = 1;
				SampRate_V_V = 1;
				VQtTable = (short *)qt_table[*(lp + 10)];
			}
			lp += llength;
			break;

	    case M_DHT:
			llength = MAKEWORD(*(lp + 1), *lp);
			if (llength < 0xd0) {
				huftab1 = (short)(*(lp + 2)) >> 4;
				huftab2 = (short)(*(lp + 2)) & 0x0f;
				huftabindex = huftab1*2 + huftab2;
				lptemp = lp + 3;
				for (i = 0; i < 16; i++)
					code_len_table[huftabindex][i] = (short)(*(lptemp++));

				j = 0;
				for (i = 0; i < 16; i++) {
					if (code_len_table[huftabindex][i] != 0) {
						k = 0;
						while (k < code_len_table[huftabindex][i]) {
							code_value_table[huftabindex][k+j] = (short)(*(lptemp++));
							k++;
						}
						j += k;
					}
				}

				i = 0;
				while (code_len_table[huftabindex][i] == 0)
					i++;

				for (j = 0; j < i; j++) {
					huf_min_value[huftabindex][j] = 0;
					huf_max_value[huftabindex][j] = 0;
				}

	            huf_min_value[huftabindex][i] = 0;
	            huf_max_value[huftabindex][i] = code_len_table[huftabindex][i] - 1;

				for (j = i + 1;j < 16; j++) {
					huf_min_value[huftabindex][j] = (huf_max_value[huftabindex][j - 1] + 1) << 1;
					huf_max_value[huftabindex][j] = huf_min_value[huftabindex][j] + code_len_table[huftabindex][j] - 1;
				}

				code_pos_table[huftabindex][0] = 0;
				for (j = 1; j < 16; j++)
					code_pos_table[huftabindex][j] = code_len_table[huftabindex][j - 1] + code_pos_table[huftabindex][j - 1];

				lp += llength;
			} else {
				hf_table_index = *(lp + 2);
				lp += 2;
				while (hf_table_index != 0xff) {
					huftab1=(short)hf_table_index >> 4;
					huftab2=(short)hf_table_index & 0x0f;
					huftabindex = huftab1 * 2 + huftab2;
					lptemp = lp + 1;
					ccount = 0;
					for (i = 0; i < 16; i++) {
						code_len_table[huftabindex][i] = (short)(*(lptemp++));
						ccount += code_len_table[huftabindex][i];
					}

					ccount += 17;
	                j = 0;
					for (i = 0; i < 16; i++) {
						if(code_len_table[huftabindex][i] != 0) {
							k = 0;
							while (k < code_len_table[huftabindex][i]) {
								code_value_table[huftabindex][k + j] = (short)(*(lptemp++));
								k++;
							}
							j += k;
						}
					}

					i = 0;
					while (code_len_table[huftabindex][i] == 0)
						i++;

					for (j = 0; j < i; j++) {
						huf_min_value[huftabindex][j] = 0;
						huf_max_value[huftabindex][j] = 0;
					}

					huf_min_value[huftabindex][i] = 0;
					huf_max_value[huftabindex][i] = code_len_table[huftabindex][i] - 1;

					for (j = i + 1; j < 16; j++) {
						huf_min_value[huftabindex][j] = (huf_max_value[huftabindex][j - 1] + 1) << 1;
						huf_max_value[huftabindex][j] = huf_min_value[huftabindex][j] + code_len_table[huftabindex][j] - 1;
					}

					code_pos_table[huftabindex][0] = 0;
					for ( j = 1; j < 16; j++)
						code_pos_table[huftabindex][j] = code_len_table[huftabindex][j - 1] + code_pos_table[huftabindex][j - 1];

					lp += ccount;
					hf_table_index = *lp;
				}  //while
			}  //else
			break;

		case M_DRI:
			llength = MAKEWORD(*(lp + 1), *lp);
			restart = MAKEWORD(*(lp + 3), *(lp + 2));
			lp += llength;
			break;

		case M_SOS:
			llength = MAKEWORD(*(lp + 1), *lp);
			comnum = *(lp + 2);
			if(comnum != comp_num)
				return -1;
			lptemp = lp + 3;
			for (i = 0; i < comp_num; i++) {
				if(*lptemp == comp_index[0]) {
					YDcIndex = (*(lptemp+1)) >> 4;	//Y
					YAcIndex = ((*(lptemp + 1)) & 0x0f) + 2;
				} else {
					UVDcIndex = (*(lptemp + 1)) >> 4;   //U,V
					UVAcIndex = ((*(lptemp + 1)) & 0x0f) + 2;
				}
				lptemp += 2;
			}
			lp += llength;
			finish = true;
			break;

		case M_EOI:
			return -1;

		default:
			if ((id & 0xf0) != 0xd0) {
				llength = MAKEWORD(*(lp + 1), *lp);
				lp += llength;
			} else
				lp += 2;
			break;

		}  //switch
	} //while

	creat_bmphead();

	return 0;
}

void fast_idct_init()
{
	short i;
	iclp = iclip + 512;

	for ( i= -512; i < 512; i++)
		iclp[i] = (i < -256) ? -256 : ((i > 255) ? 255 : i);
}

__u8 read_u8()
{
	__u8 i;
	i = *(lp++);
	if (i == 0xff)
		lp++;
	bit_pos = 8;
	cur_byte = i;
	return i;
}

int decode_element()
{
	int thiscode, tempcode;
	__u16 temp, valueex;
	short codelen;
	__u8 hufexu8, runsize, tempsize, sign;
	__u8 newu8, lastu8;

	if( bit_pos >= 1 ) {
		bit_pos--;
		thiscode = (__u8)cur_byte >> bit_pos;
		cur_byte = cur_byte&And[bit_pos];
	} else {
		lastu8 = read_u8();
		bit_pos--;
		newu8 = cur_byte&And[bit_pos];
		thiscode = lastu8 >> 7;
		cur_byte = newu8;
	}

	codelen = 1;

	while ((thiscode < huf_min_value[HufTabIndex][codelen - 1])||
		(code_len_table[HufTabIndex][codelen - 1] == 0)||
		(thiscode>huf_max_value[HufTabIndex][codelen-1]))
	{
		if(bit_pos >= 1) {
			bit_pos--;
			tempcode = (__u8)cur_byte>>bit_pos;
			cur_byte = cur_byte&And[bit_pos];
		} else {
			lastu8 = read_u8();
			bit_pos--;
			newu8 = cur_byte&And[bit_pos];
			tempcode = (__u8)lastu8 >> 7;
			cur_byte = newu8;
		}

		thiscode = (thiscode << 1) + tempcode;
		codelen++;
		if (codelen > 16)
			return -1;
	}  //while

	temp = thiscode-huf_min_value[HufTabIndex][codelen - 1] + code_pos_table[HufTabIndex][codelen - 1];
	hufexu8 = (__u8)code_value_table[HufTabIndex][temp];
	rrun = (short)(hufexu8 >> 4);
	runsize = hufexu8 & 0x0f;
	if (runsize == 0) {
		vvalue = 0;
		return 0;
	}
	tempsize = runsize;
	if(bit_pos >= runsize) {
		bit_pos -= runsize;
		valueex = (__u8)cur_byte >> bit_pos;
		cur_byte = cur_byte&And[bit_pos];
	} else {
		valueex = cur_byte;
		tempsize -= bit_pos;
		while (tempsize > 8) {
			lastu8 = read_u8();
			valueex =(valueex << 8) + (__u8)lastu8;
			tempsize -= 8;
		}  //while

		lastu8 = read_u8();
		bit_pos -= tempsize;
		valueex = (valueex << tempsize) + (lastu8 >> bit_pos);
		cur_byte = lastu8&And[bit_pos];
	}  //else

	sign = valueex >> (runsize-1);
	if (sign)
		vvalue = valueex;
	else {
		valueex = valueex ^ 0xffff;
		temp = 0xffff << runsize;
		vvalue =- (short)(valueex ^ temp);
	}

	return 0;
}

int HufBlock(__u8 dchufindex, __u8 achufindex)
{
	short count = 0;
	short i;
	int ret;

	//dc
	HufTabIndex = dchufindex;
	ret = decode_element();
	if (ret != 0)
		return ret;

	BlockBuffer[count++] = vvalue;
	//ac
	HufTabIndex = achufindex;
	while (count < 64) {
		ret = decode_element();
		if (ret != 0)
			return ret;

		if ((rrun == 0) && (vvalue == 0)) {
			for (i = count; i < 64; i++)
				BlockBuffer[i] = 0;
			count = 64;
		} else {
			for (i = 0;i < rrun; i++)
				BlockBuffer[count++] = 0;
			BlockBuffer[count++] = vvalue;
		}
	}

	return 0;
}

int dec_mcu_block()
{
	short *lpMCUBuffer;
	short i, j;
	int ret;

	if (interval_flag) {
		lp += 2;
		ycoef = ucoef = vcoef = 0;
		bit_pos = 0;
		cur_byte = 0;
	}

	switch(comp_num) {
	case 3:
		lpMCUBuffer = MCUBuffer;
		for ( i = 0; i < SampRate_Y_H * SampRate_Y_V; i++) {  //Y
			ret = HufBlock(YDcIndex, YAcIndex);
			if (ret != 0)
				return ret;
			BlockBuffer[0] = BlockBuffer[0] + ycoef;
			ycoef = BlockBuffer[0];
			for (j = 0; j < 64; j++)
				*lpMCUBuffer++ = BlockBuffer[j];
		}

		for (i = 0; i < SampRate_U_H * SampRate_U_V; i++) {  //U
			ret = HufBlock(UVDcIndex, UVAcIndex);
			if (ret != 0)
				return ret;
			BlockBuffer[0] = BlockBuffer[0] + ucoef;
			ucoef = BlockBuffer[0];
			for ( j = 0; j < 64; j++)
				*lpMCUBuffer++ = BlockBuffer[j];
		}

		for ( i = 0; i < SampRate_V_H * SampRate_V_V; i++) {  //V
			ret = HufBlock(UVDcIndex, UVAcIndex);
			if (ret != 0)
				return ret;
			BlockBuffer[0] = BlockBuffer[0] + vcoef;
			vcoef = BlockBuffer[0];
			for (j = 0;j < 64; j++)
				*lpMCUBuffer++ = BlockBuffer[j];
		}
		break;

	case 1:
		lpMCUBuffer = MCUBuffer;
		ret = HufBlock(YDcIndex, YAcIndex);
		if (ret != 0)
			return ret;
		BlockBuffer[0] = BlockBuffer[0] + ycoef;
		ycoef = BlockBuffer[0];
		for (j = 0; j < 64; j++)
			*lpMCUBuffer++ = BlockBuffer[j];
		for (i = 0; i < 128; i++)
			*lpMCUBuffer++ = 0;
		break;

	default:
		return -1;
	}

	return 0;
}

void idctrow(int * blk)
{
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;
	//intcut
	if (!((x1 = blk[4]<<11) | (x2 = blk[6]) | (x3 = blk[2]) |
		(x4 = blk[1]) | (x5 = blk[7]) | (x6 = blk[5]) | (x7 = blk[3])))
	{
		blk[0] = blk[1] = blk[2] = blk[3] = blk[4] = blk[5] = blk[6] = blk[7] = blk[0] << 3;
		return;
	}
	x0 = (blk[0] << 11) + 128; // for proper rounding in the fourth stage
	//first stage
	x8 = W7 * (x4 + x5);
	x4 = x8 + (W1 - W7) * x4;
	x5 = x8 - (W1 + W7) * x5;
	x8 = W3 * (x6 + x7);
	x6 = x8 - (W3 - W5) * x6;
	x7 = x8 - (W3 + W5) * x7;
	//second stage
	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6 * (x3 + x2);
	x2 = x1 - (W2 + W6) * x2;
	x3 = x1 + (W2 - W6) * x3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;
	//third stage
	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181 * (x4 + x5) + 128) >> 8;
	x4 = (181 * (x4 - x5) + 128) >> 8;
	//fourth stage
	blk[0] = (x7 + x1) >> 8;
	blk[1] = (x3 + x2) >> 8;
	blk[2] = (x0 + x4) >> 8;
	blk[3] = (x8 + x6) >> 8;
	blk[4] = (x8 - x6) >> 8;
	blk[5] = (x0 - x4) >> 8;
	blk[6] = (x3 - x2) >> 8;
	blk[7] = (x7 - x1) >> 8;
}

//////////////////////////////////////////////////////////////////////////////
void idctcol(int * blk)
{
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;
	//intcut
	if (!((x1 = (blk[8*4] << 8)) | (x2 = blk[8*6]) | (x3 = blk[8*2]) |
		(x4 = blk[8 * 1]) | (x5 = blk[8 * 7]) | (x6 = blk[8 * 5]) | (x7 = blk[8 * 3])))
	{
		blk[8 * 0] = blk[8 * 1] = blk[8 * 2] = blk[8 * 3] = blk[8 * 4] = blk[8 * 5]
			 =blk[8 * 6] = blk[8 * 7] = iclp[(blk[8 * 0] + 32) >> 6];
		return;
	}
	x0 = (blk[8 * 0] << 8) + 8192;
	//first stage
	x8 = W7 * (x4+x5) + 4;
	x4 = (x8 + (W1 - W7) * x4) >> 3;
	x5 = (x8 - (W1 + W7) * x5) >> 3;
	x8 = W3 * (x6 + x7) + 4;
	x6 = (x8 - (W3 - W5) * x6) >> 3;
	x7 = (x8 - (W3 + W5) * x7) >> 3;
	//second stage
	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6 * (x3 + x2) + 4;
	x2 = (x1 - (W2 + W6) * x2) >> 3;
	x3 = (x1 + (W2 - W6) * x3) >> 3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;
	//third stage
	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181 * (x4 + x5) + 128) >> 8;
	x4 = (181 * (x4 - x5) + 128) >> 8;
	//fourth stage
	blk[8 * 0] = iclp[(x7 + x1) >> 14];
	blk[8 * 1] = iclp[(x3 + x2) >> 14];
	blk[8 * 2] = iclp[(x0 + x4) >> 14];
	blk[8 * 3] = iclp[(x8 + x6) >> 14];
	blk[8 * 4] = iclp[(x8 - x6) >> 14];
	blk[8 * 5] = iclp[(x0 - x4) >> 14];
	blk[8 * 6] = iclp[(x3 - x2) >> 14];
	blk[8 * 7] = iclp[(x7 - x1) >> 14];
}

void Fast_IDCT(int * block)
{
	short i;

	for ( i = 0; i < 8; i++)
		idctrow(block + 8 * i);

	for ( i = 0; i < 8; i++)
		idctcol(block + i);
}

void IQtIZzBlock(short *s, int *d, short flag)
{
	short i, j;
	short tag;
	short *pQt = NULL;
	int buffer2[8][8];
	int *buffer1;
	short offset = 0;

	switch(flag) {
	case 0:
		pQt = YQtTable;
		offset = 128;
		break;

	case 1:
		pQt = UQtTable;
		offset = 0;
		break;

	case 2:
		pQt = VQtTable;
		offset = 0;
		break;
	}

	for ( i = 0; i < 8; i++) {
		for ( j = 0; j < 8; j++) {
			tag = Zig_Zag[i][j];
			buffer2[i][j] = (int)s[tag] * (int)pQt[tag];
		}
	}

	buffer1 = (int *)buffer2;
	Fast_IDCT(buffer1);
	for ( i = 0; i < 8; i++) {
		for ( j = 0; j < 8; j++)
			d[i*8+j] = buffer2[i][j] + offset;
	}
}

void IQtIZzMCUComponent(short flag)
{
	short H = 0, VV = 0;
	short i, j;
	int *pQtZzMCUBuffer = NULL;
	short  *pMCUBuffer;

	switch(flag) {
	case 0:
		H  = SampRate_Y_H;
		VV = SampRate_Y_V;
		pMCUBuffer = MCUBuffer;
		pQtZzMCUBuffer = QtZzMCUBuffer;
		break;

	case 1:
		H  = SampRate_U_H;
		VV = SampRate_U_V;
		pMCUBuffer = MCUBuffer+Y_in_MCU * 64;
		pQtZzMCUBuffer = QtZzMCUBuffer+Y_in_MCU * 64;
		break;

	case 2:
		H = SampRate_V_H;
		VV = SampRate_V_V;
		pMCUBuffer = MCUBuffer + (Y_in_MCU+U_in_MCU) * 64;
		pQtZzMCUBuffer = QtZzMCUBuffer + (Y_in_MCU+U_in_MCU) * 64;
		break;

	}

	for ( i = 0; i < VV; i++) {
		for ( j = 0; j < H; j++)
			IQtIZzBlock(pMCUBuffer + (i * H + j) * 64, pQtZzMCUBuffer + (i * H + j) * 64,flag);
	}
}

void  getyuv(short flag)
{
	short	H = 0, VV = 0;
	short	i,j,k,h;
	int		*buf = NULL;
	int		*pQtZzMCU = NULL;

	switch(flag) {
	case 0:
		H = SampRate_Y_H;
		VV = SampRate_Y_V;
		buf = Y;
		pQtZzMCU = QtZzMCUBuffer;
		break;

	case 1:
		H = SampRate_U_H;
		VV = SampRate_U_V;
		buf = U;
		pQtZzMCU = QtZzMCUBuffer+Y_in_MCU * 64;
		break;

	case 2:
		H = SampRate_V_H;
		VV = SampRate_V_V;
		buf = V;
		pQtZzMCU = QtZzMCUBuffer+(Y_in_MCU+U_in_MCU) * 64;
		break;

	}

	for (i = 0; i < VV; i++)
		for(j = 0; j < H; j++)
			for(k = 0; k < 8; k++)
				for(h = 0; h < 8; h++)
					buf[(i * 8 + k) * SampRate_Y_H * 8 + j * 8 + h] = *pQtZzMCU++;

}

void store_buf()
{
	short i, j;
	__u8  *lpbmp;
	__u8 R, G, B;
	int y, u, v, rr, gg, bb;

	line_ubytes = (__u32)WIDTHBYTES(img_w * bi.biBitCount);

	for (i = 0; i < SampRate_Y_V * 8; i++) {
		if ((size_i + i) < img_h) {
			lpbmp = ((__u8*)lpPtr + (__u32)(img_h - size_i - i - 1) * line_ubytes + size_j * 3);

			for (j = 0; j < SampRate_Y_H * 8; j++) {
				if ((size_j + j) < img_w) {
				    y = Y[i * 8 * SampRate_Y_H + j];
				    u = U[(i / V_YtoU) * 8 * SampRate_Y_H + j / H_YtoU];
				    v = V[(i / V_YtoV) * 8 * SampRate_Y_H + j / H_YtoV];
				    rr = ((y << 8) + 18*u + 367 * v) >> 8;
				    gg = ((y << 8) - 159 * u - 220 * v) >> 8;
				    bb = ((y << 8) + 411 * u - 29 * v) >> 8;
				    R = (__u8)rr;
				    G = (__u8)gg;
				    B = (__u8)bb;
				    if (rr & 0xffffff00){if (rr > 255) R = 255; else if (rr < 0) R = 0;}
				    if (gg & 0xffffff00){if (gg > 255) G = 255; else if (gg < 0) G = 0;}
				    if (bb & 0xffffff00){if (bb > 255) B = 255; else if (bb < 0) B = 0;}
				    *lpbmp++ = B;
				    *lpbmp++ = G;
				    *lpbmp++ = R;
				} else
	                break;
			}
		} else
	       break;
	}
}

int decode(__u8* rgbbuf)
{
	int ret;

	Y_in_MCU = SampRate_Y_H * SampRate_Y_V;
	U_in_MCU = SampRate_U_H * SampRate_U_V;
	V_in_MCU = SampRate_V_H*SampRate_V_V;
	H_YtoU = SampRate_Y_H / SampRate_U_H;
	V_YtoU = SampRate_Y_V / SampRate_U_V;
	H_YtoV = SampRate_Y_H / SampRate_V_H;
	V_YtoV = SampRate_Y_V / SampRate_V_V;

	fast_idct_init();
	lpPtr = (char *)rgbbuf;

	while ((ret = dec_mcu_block()) == 0) {
		interval++;
		if ((restart) && (interval % restart == 0))
			interval_flag = true;
		else
			interval_flag = false;

		IQtIZzMCUComponent(0);
		IQtIZzMCUComponent(1);
		IQtIZzMCUComponent(2);

		getyuv(0);
		getyuv(1);
		getyuv(2);

		store_buf();

		size_j += SampRate_Y_H * 8;

		if(size_j >= img_w) {
			size_j = 0;
			size_i += SampRate_Y_V * 8;
		}

		if ((size_j == 0) && (size_i >= img_h))
			break;
	}

	return ret;
}

int jpeg2bmp_decode(struct djpeg_opts *djpeg2bmp)
{
	int ret;

	init_table();

	if(0 != init_tag(djpeg2bmp->jpegbuf)) {
	    printf("init_tag failed\n");
		return -1;
	}

	djpeg2bmp->imgbf = get_bmpfileheader();
	djpeg2bmp->imgbi = get_bmpinfoheader();

	printf("image width:%ld, height:%ld, bmp file size:%d, rgb data size:%d\n", djpeg2bmp->imgbi->biWidth, djpeg2bmp->imgbi->biHeight, djpeg2bmp->imgbf->bfSize, djpeg2bmp->imgbf->bfSize - djpeg2bmp->imgbf->bfOffBits);

	djpeg2bmp->bmpbuf = malloc(djpeg2bmp->imgbf->bfSize);
	djpeg2bmp->rgbdata = djpeg2bmp->bmpbuf + djpeg2bmp->imgbf->bfOffBits;
	ret = decode(djpeg2bmp->rgbdata);

	///////////////////////////////////////////////

	if (0 == ret) {
	    printf("decode jpeg ok!\n");
		memcpy(djpeg2bmp->bmpbuf, djpeg2bmp->imgbf, sizeof(BITMAPFILEHEADER));
	    memcpy(djpeg2bmp->bmpbuf + sizeof(BITMAPFILEHEADER), djpeg2bmp->imgbi, djpeg2bmp->imgbi->biSize);
	} else
	    printf("decoder jpeg error!\n");

	return 0;
}
