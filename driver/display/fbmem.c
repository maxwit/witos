#include <fb.h>
#include <fs.h>
#include <fcntl.h>
#include <init.h>
#include <errno.h>
#include <string.h>
#include <malloc.h>
#include <graphic/display.h>

#define MAX_FB   4
#define FB_MAJOR 29

static struct fb_info *g_fb_info[MAX_FB];

static int fb_open(struct file *, struct inode *);
static int fb_close(struct file *);
static ssize_t fb_read(struct file *, void *, size_t, loff_t *);
static ssize_t fb_write(struct file *, const void *, size_t, loff_t *);
static int fb_ioctl(struct file *, int, unsigned long);
static loff_t fb_lseek(struct file *, loff_t, int);

const struct file_operations fb_fops = {
	.open  = fb_open,
	.close = fb_close,
	.read  = fb_read,
	.write = fb_write,
	.ioctl = fb_ioctl,
	.lseek = fb_lseek,
};

static int fb_open(struct file *fp, struct inode *inode)
{
	unsigned int minor = iminor(inode);
	struct fb_info *fb;

	if (minor >= MAX_FB)
		return -EINVAL;

	fb = g_fb_info[minor];
	if (!fb)
		return -ENODEV;

	fp->private_data = fb;

	return 0;
}

static int fb_close(struct file *fp)
{
	return 0;
}

static ssize_t fb_read(struct file *fp, void *buff, size_t size, loff_t *loff)
{
	return -EIO;
}

static ssize_t fb_write(struct file *fp, const void *buff,
	size_t size, loff_t *loff)
{
	size_t real_size;
	struct fb_info *fb = fp->private_data;

	// if null

	if (*loff + size > fb->fix.smem_len)
		real_size = fb->fix.smem_len - *loff;
	else
		real_size = size;

	memcpy(fb->screenbase + *loff, buff, real_size);
	*loff += real_size;

	return real_size;
}

static int fb_ioctl(struct file *fp, int cmd, unsigned long arg)
{
	switch (cmd) {
	default:
		return -EINVAL;
	}

	return 0;
}

static loff_t fb_lseek(struct file *fp, loff_t loff, int whence)
{
	struct fb_info *fb = fp->private_data;

	switch (whence) {
	case SEEK_SET:
		fp->f_pos = loff;
		break;

	case SEEK_CUR:
		fp->f_pos += loff;
		break;

	case SEEK_END:
		fp->f_pos = fb->fix.smem_len - loff;
		break;

	default:
		return -ENOTSUPP;
	}

	return 0;
}

struct fb_info *framebuffer_alloc(size_t size)
{
	struct fb_info *fb;
	void *p;

	fb = p = malloc(sizeof(*fb) + size);
	// if p ...
	p += sizeof(*fb);

	fb->par = p;

	return fb;
}

int framebuffer_register(struct fb_info *fb)
{
	int i;
	struct fb_fix_screeninfo *fix = &fb->fix;

	for (i = 0; i < MAX_FB; i++)
		if (!g_fb_info[i])
			break;

	if (MAX_FB == i)
		return -EBUSY;

	g_fb_info[i] = fb;

	memset(fb->screenbase, 0x0, fix->smem_len);

	device_create(MKDEV(FB_MAJOR, i), "fb%d", i);
	// ...

	return 0;
}

static int __init fbmem_init(void)
{
	int ret;

	ret = register_chrdev(FB_MAJOR, &fb_fops, "fb");
	// ...

	return ret;
}

subsys_init(fbmem_init);
