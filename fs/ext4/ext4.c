#include <stdio.h>
#include <init.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <block.h>
#include <dirent.h>
#include <fs.h>
#include <fs/ext4_fs.h>
#include <fs/ext4.h>

#define MAX_MNT_LEN 256
#define SECT_SIZE   (1 << 9) // fixme

static struct dentry *ext4_lookup(struct inode *parent, struct dentry *dentry,
						struct nameidata *nd);

static const struct inode_operations ext4_reg_inode_operations = {
};

static const struct inode_operations ext4_dir_inode_operations = {
	.lookup = ext4_lookup,
};

static int ext4_open(struct file *fp, struct inode *inode);
static int ext4_close(struct file *fp);
static ssize_t ext4_read(struct file *fp, void *buff, size_t size, loff_t *off);
static ssize_t ext4_write(struct file *fp, const void *buff, size_t size, loff_t *off);

static const struct file_operations ext4_reg_file_operations = {
	.open  = ext4_open,
	.close = ext4_close,
	.read  = ext4_read,
	.write = ext4_write,
};

static int ext4_readdir(struct file *fp, void *dirent, filldir_t filldir);

static const struct file_operations ext4_dir_file_operations = {
	.readdir = ext4_readdir,
};

static struct inode *ext4_alloc_inode(struct super_block *sb)
{
	struct ext4_inode_info *eii;

	eii = zalloc(sizeof(*eii));
	if (!eii)
		return NULL;

	eii->vfs_inode.i_sb = sb; // fixme

	return &eii->vfs_inode;
}

static ssize_t ext4_read_block(struct super_block *sb, void *buff, int blk_no, size_t off, size_t size)
{
	ssize_t ret, start_blk;
	struct bio *bio;
	struct ext4_sb_info *e4_sbi = sb->s_fs_info;
	struct ext4_super_block *e4_sb;

	e4_sb = &e4_sbi->e4_sb;
	start_blk = blk_no << (e4_sb->s_log_block_size + 1);

	bio = bio_alloc();
	if (!bio)
		return -ENOMEM;

	bio->size = (off + size + SECT_SIZE - 1) & ~(SECT_SIZE - 1);
	bio->data = malloc(bio->size);
	if (!bio->data) {
		ret = -ENOMEM;
		goto L1;
	}

	bio->bdev = sb->s_bdev;
	bio->sect = start_blk;
	submit_bio(READ, bio);

	memcpy(buff, bio->data + off, size);
	ret = size;

	free(bio->data);
L1:
	bio_free(bio);
	return ret;
}

static size_t get_start_layer(size_t start_block, size_t index_per_block)
{
	if(start_block < EXT4_IND_BLOCK)
		return 0;
	else if(start_block < EXT4_IND_BLOCK + index_per_block)
		return 1;
	else if(start_block < EXT4_IND_BLOCK + index_per_block +
			index_per_block * index_per_block)
		return 2;
	else
		return 3;
}

static size_t get_ind_block(struct super_block *sb,
			struct ext4_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	struct ext4_sb_info *e4_sbi = sb->s_fs_info;
	size_t block_size = 1 << (e4_sbi->e4_sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	size_t i;

	start_block = start_block < 0 ? 0 : start_block;

	if(len <= 0)
		return 0;

	ext4_read_block(sb, buff, inode->i_block[EXT4_IND_BLOCK], 0, block_size);

	for(i = 0; i < len && i < index_per_block - start_block; i++)
		block_indexs[i] = buff[i + start_block];

	return i;
}

static size_t get_dind_block(struct super_block *sb,
			struct ext4_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	struct ext4_sb_info *e4_sbi = sb->s_fs_info;
	size_t block_size = 1 << (e4_sbi->e4_sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	int i = 0, j, k;

	if(len <= 0)
		return 0;

	ext4_read_block(sb, buff, inode->i_block[EXT4_DIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	k = start_block % index_per_block;

	for(j = start_block / index_per_block; j < index_per_block; j++) {
		ext4_read_block(sb, dbuff, buff[j], 0, block_size);

		while (i < len && k < index_per_block)
			block_indexs[i++] = dbuff[k++];

		if(i >= len)
			return i;

		k = 0;
	}

	return i;
}

static size_t get_tind_block(struct super_block *sb,
			struct ext4_inode *inode, ssize_t start_block, __le32 block_indexs[], size_t len)
{
	size_t block_size = sb->s_blocksize;
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	__le32 tbuff[index_per_block];
	int i = 0 /*? fixme*/, j, k, h;

	if(len <= 0)
		return 0;

	ext4_read_block(sb, buff, inode->i_block[EXT4_TIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	h = start_block % index_per_block;

	for(j = start_block / (index_per_block * index_per_block); j < index_per_block; j++) {
		ext4_read_block(sb, dbuff, buff[j], 0, block_size);

		for(k = start_block / index_per_block; k < index_per_block; k++) {
			ext4_read_block(sb, tbuff, dbuff[k], 0, block_size);

			for(; i < len && h < index_per_block; i++, h++)
				block_indexs[i] = tbuff[h];

			if(i >= len)
				return i;

			h = 0;
		}
	}

	return i;
}

static int get_block_indexs(struct super_block *sb,
			struct ext4_inode *inode,size_t start_block, __le32 block_indexs[], size_t len)
{
	struct ext4_sb_info *e4_sbi = sb->s_fs_info;
	size_t index_per_block = (1 << (e4_sbi->e4_sb.s_log_block_size + 10)) / sizeof(__le32);
	size_t start_layer;
	size_t i = 0;

	if (NULL == inode)
		return -1;

	if(len <= 0)
		return 0;

	start_layer = get_start_layer(start_block, index_per_block);

	if (0 == start_layer) {
		for(i = 0; i < len && i < EXT4_IND_BLOCK - start_block; i++)
			block_indexs[i] = inode->i_block[i + start_block];

		start_layer = -1;
	}

	if(i < len && (start_layer == 1 || start_layer == -1)) {
		i += get_ind_block(sb, inode, start_block - EXT4_IND_BLOCK,
							block_indexs + i, len - i);

		start_layer = -1;
	}

	if(i < len && (start_layer == 2 || start_layer == -1)) {
		i += get_dind_block(sb, inode, start_block - EXT4_IND_BLOCK - index_per_block
							, block_indexs + i, len - i);

		start_layer = -1;
	}

	if(i < len && (start_layer == 3 || start_layer == -1)) {
		i += get_tind_block(sb, inode, start_block - EXT4_IND_BLOCK - index_per_block
							- index_per_block * index_per_block, block_indexs + i, len - i);
	}

	return i;
}

static struct ext4_inode *ext4_get_inode(struct super_block *sb, int ino)
{
	int grp_no, ino_no, blk_no, count;
	struct ext4_sb_info *e4_sbi = sb->s_fs_info;
	struct ext4_inode *e4_in;
	struct ext4_super_block *e4_sb;
	struct ext4_group_desc *gde;

	ino--;
	e4_sb = &e4_sbi->e4_sb;

	count = (1 << (e4_sb->s_log_block_size + 10)) / e4_sb->s_inode_size;

	grp_no = ino / e4_sb->s_inodes_per_group;
	gde = &e4_sbi->gdt[grp_no];

	ino_no = ino % e4_sb->s_inodes_per_group;
	blk_no = ino_no / count;
	ino_no = ino_no % count;

	e4_in = malloc(e4_sb->s_inode_size);
	// if

	ext4_read_block(sb, e4_in, gde->bg_inode_table + blk_no, ino_no * e4_sb->s_inode_size, e4_sb->s_inode_size);

	return e4_in;
}

bool ext4_check_fstype(void *buff, size_t size)
{
	struct ext4_super_block *e4_sb = buff;

	if (size >= sizeof(struct ext4_super_block) && 0xEF53 == e4_sb->s_magic)
		return true;

	return false;
}

struct inode *ext4_iget(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;
	struct ext4_inode *e4_in;
	struct ext4_inode_info *e4_ini;

	e4_in = ext4_get_inode(sb, ino);
	// ...

	inode = ext4_alloc_inode(sb);
	if (!inode) {
		// ...
		return NULL;
	}

	inode->i_ino  = ino;
	inode->i_mode = le16_to_cpu(e4_in->i_mode);
	inode->i_size = le32_to_cpu(e4_in->i_size);

	e4_ini = EXT4_I(inode);
	e4_ini->i_e4in = e4_in; // fixme

	if (S_ISREG(inode->i_mode)) {
		inode->i_op = &ext4_reg_inode_operations;
		inode->i_fop = &ext4_reg_file_operations;
	} else if (S_ISDIR(inode->i_mode)) {
		inode->i_op = &ext4_dir_inode_operations;
		inode->i_fop = &ext4_dir_file_operations;
	} else {
		BUG();
	}

	return inode;
}

static int ext4_fill_super(struct super_block *sb)
{
	int ret;
	int group_count;
	unsigned int bpg; // blocks per groups
	char buff[KB(1)];
	struct ext4_sb_info *e4_sbi;
	struct ext4_super_block *e4_sb;
	struct ext4_group_desc *gdt;
	struct bio *bio;

	bio = bio_alloc();
	if (!bio)
		return -ENOMEM;

	bio->bdev = sb->s_bdev;
	bio->sect = 1024 / SECT_SIZE;
	bio->size = sizeof(buff);
	bio->data = buff;
	submit_bio(READ, bio);
	// TODO: check flags here
	bio_free(bio);

	if (ext4_check_fstype(buff, sizeof(struct ext4_super_block)) == false) {
		GEN_DBG("Invalid EXT4 magic number!\n"); // ((struct ext4_super_block *)buff)->s_magic);
		ret = -EINVAL;
		goto L1;
	}

	e4_sbi = zalloc(sizeof(*e4_sbi));
	if (!e4_sbi) {
		ret = -ENOMEM;
		goto L1;
	}

#if 1 // fixme
	e4_sb = &e4_sbi->e4_sb;
	memcpy(e4_sb, buff, sizeof(*e4_sb));
#endif

	sb->s_fs_info = e4_sbi;
	sb->s_blocksize = 1024 << e4_sb->s_log_block_size;

	bpg = e4_sb->s_blocks_per_group;
	group_count = (e4_sb->s_blocks_count + bpg - 1) / bpg;
	DPRINT("super block information:\n"
		"label = \"%s\", inode size = %d, block size = %d\n",
		e4_sb->s_volume_name[0] ? e4_sb->s_volume_name : "<N/A>",
		e4_sb->s_inode_size, sb->s_blocksize);

	gdt = malloc(group_count * sizeof(struct ext4_group_desc));
	if (NULL == gdt) {
		ret = -ENOMEM;
		goto L2;
	}
	e4_sbi->gdt = gdt;

	ext4_read_block(sb, gdt, e4_sb->s_first_data_block + 1, 0,
		group_count * sizeof(struct ext4_group_desc));

	DPRINT("group descrition:\n"
		"block groups = %d, free blocks = %d, free inodes = %d\n",
		group_count, gdt->bg_free_blocks_count, gdt->bg_free_inodes_count);


	return 0;

L2:
	free(e4_sbi);
L1:
	return ret;
}

static int ext4_read_super(struct super_block *sb, void *data, int flags)
{
	int ret;
	struct inode *in;
	struct dentry *root;

	ret = ext4_fill_super(sb);
	if (ret < 0) {
		// ...
		return ret;
	}

	in = ext4_iget(sb, 2);
	if (!in) {
		// ...
		return -EINVAL;
	}

	root = d_make_root(in);
	if (!root)
		return EINVAL;

	sb->s_root = root;

	return 0;
}

static struct dentry *ext4_mount(struct file_system_type *fs,
			    int flags, const char *dev_name, void *data)
{
	return mount_bdev(fs, flags, dev_name, data, ext4_read_super);
}

// fixme
static void ext4_kill_sb(struct super_block *sb)
{
}

static unsigned long ext4_inode_by_name(struct inode *inode, struct qstr *unit)
{
	struct super_block *sb = inode->i_sb;
	struct ext4_sb_info *e4_sbi;
	struct ext4_inode_info *e4_ini = EXT4_I(inode);
	struct ext4_dir_entry_2 *e4_de;
	struct ext4_inode *parent = e4_ini->i_e4in;
	char buff[parent->i_size];
	size_t len = 0;
	int blocks, i;
	size_t block_size;

	e4_sbi = sb->s_fs_info;
	block_size = 1024 << e4_sbi->e4_sb.s_log_block_size;

	blocks = (parent->i_size + block_size - 1) / block_size;
	__le32 block_indexs[blocks];

	get_block_indexs(sb, parent, 0, block_indexs, blocks);

	for (i = 0; i < blocks; i++) {
		ext4_read_block(sb, buff, block_indexs[i], 0, block_size);

		e4_de = (struct ext4_dir_entry_2 *)buff;

		while (e4_de->rec_len > 0 && len < parent->i_size && len < (i + 1) * block_size) {
			e4_de->name[e4_de->name_len] = '\0';

			DPRINT("%s: inode = %d, e4_de size = %d, name size = %d, block = %d\n",
				e4_de->name, e4_de->inode, e4_de->rec_len, e4_de->name_len, i);

			if (unit->len == e4_de->name_len && \
				!strncmp(e4_de->name, unit->name, e4_de->name_len))
				return  e4_de->inode;

			e4_de = (struct ext4_dir_entry_2 *)((char *)e4_de + e4_de->rec_len);
			len += e4_de->rec_len;
		}
	}

	GEN_DBG("\"%s\" not found!\n", unit->name);

	return 0;
}

static int ext4_open(struct file *fp, struct inode *inode)
{
	return 0;
}

static int ext4_close(struct file *fp)
{
	return 0;
}

static ssize_t ext4_read(struct file *fp, void *buff, size_t size, loff_t *off)
{
	ssize_t i, len;
	size_t blocks;
	size_t offset, real_size, block_size;
	struct dentry *de = fp->f_dentry;
	struct inode *in;
	struct super_block *sb;
	struct ext4_inode *e4_in;
	struct ext4_sb_info *e4_sbi;

	sb     = de->d_sb;
	in     = de->d_inode;
	e4_sbi = sb->s_fs_info;

	e4_in = ext4_get_inode(sb, in->i_ino);
	if (!e4_in)
		return -ENOENT;

	// fixme!!!
	if (fp->f_pos == e4_in->i_size)
		return 0;

	block_size = 1 << (e4_sbi->e4_sb.s_log_block_size + 10);
	char tmp_buff[block_size];

	real_size = min(size, e4_in->i_size - fp->f_pos);

	blocks = (real_size + (block_size - 1)) / block_size;
	__le32 block_indexs[blocks];

	get_block_indexs(sb, e4_in, fp->f_pos / block_size, block_indexs, blocks);

	offset = fp->f_pos % block_size;
	len = block_size - offset;

	for(i = 0; i < blocks; i++) {
		if(i == blocks - 1)
			len = real_size % block_size == 0 ? block_size : (real_size % block_size);

		ext4_read_block(sb, tmp_buff, block_indexs[i], offset, len);

		memcpy(buff, tmp_buff, len);
		buff += len;
		fp->f_pos += len;
		offset = 0;
		len = block_size;
	}

	return real_size;
}

static ssize_t ext4_write(struct file *fp, const void *buff, size_t size, loff_t *off)
{
	return 0;
}

static struct dentry *ext4_lookup(struct inode *parent, struct dentry *dentry, struct nameidata *nd)
{
	unsigned long ino;
	struct inode *inode;

	ino = ext4_inode_by_name(parent, &dentry->d_name);
	if (!ino) {
		// ...
		nd->ret = -ENOENT;
		return NULL;
	}

	inode = ext4_iget(parent->i_sb, ino);
	if (!inode) {
		// ...
		nd->ret = -EIO;
		return NULL; // fixme!!!
	}

	// dentry->d_inode = inode;
	d_add(dentry, inode);
	nd->ret = 0;

	return dentry;
}

static int ext4_inode_read_block(struct inode *in, int blk_no, char *blk_buf)
{
	struct ext4_inode_info *e4_info = EXT4_I(in);
	int blk_index[in->i_size / in->i_sb->s_blocksize];

	get_block_indexs(in->i_sb, e4_info->i_e4in, 0, (__le32 *)blk_index, ARRAY_ELEM_NUM(blk_index));

	ext4_read_block(in->i_sb, blk_buf, blk_index[blk_no], 0, in->i_sb->s_blocksize);

	return 0;
}

static int ext4_readdir(struct file *fp, void *dirent, filldir_t filldir)
{
	int blk_num;
	int blk_off;
	char buff[fp->f_dentry->d_inode->i_sb->s_blocksize];
	struct inode *in;
	struct ext4_dir_entry_2 *e4_de;

	in = fp->f_dentry->d_inode;

	if (fp->f_pos == in->i_size)
		return 0;

	blk_num = fp->f_pos / in->i_sb->s_blocksize;
	blk_off = fp->f_pos % in->i_sb->s_blocksize;

	ext4_inode_read_block(in, blk_num, buff);

	e4_de = (struct ext4_dir_entry_2 *)(buff + blk_off);

	if (e4_de->rec_len + fp->f_pos > in->i_size)
		return -ENODATA;

	filldir(dirent, e4_de->name, e4_de->name_len, fp->f_pos /*fixme*/,
		in->i_ino, e4_de->file_type);

	fp->f_pos += e4_de->rec_len;

	return e4_de->rec_len;
}

extern int ck_ext4_feature(uint32_t fc,uint32_t frc,uint32_t fi);
static int ext4_check_fs_type(const char *bdev_name)
{
	struct ext4_super_block *e4_sb;
	struct bio *bio;
	char buff[EXT4_SUPER_BLK_SIZE];
	struct block_device *bdev;
	uint32_t fc, frc, fi;

	bdev = bdev_get(bdev_name);
	if (NULL == bdev) {
		DPRINT("bdev %s not found!\n", bdev_name);
		return -ENODEV;
	}

	bio = bio_alloc();
	if (!bio)
		return -ENOMEM;

	bio->bdev = bdev;
	bio->sect = 1024 / SECT_SIZE;
	bio->size = sizeof(buff);
	bio->data = buff;
	submit_bio(READ, bio);
	// TODO: check flags here
	bio_free(bio);

	e4_sb = (struct ext4_super_block *)buff;

	if (0xEF53 != e4_sb->s_magic) {
		DPRINT("%s is not \"ext4\" fs!\n", bdev_name);
		return -EINVAL;
	}

	fc = e4_sb->s_feature_compat;
	fi = e4_sb->s_feature_incompat;
	frc = e4_sb->s_feature_ro_compat;

	return ck_ext4_feature(fc, frc, fi);
}


static struct file_system_type ext4_fs_type = {
	.name    = "ext4",
	.mount   = ext4_mount,
	.kill_sb = ext4_kill_sb,
	.check_fs_type = ext4_check_fs_type,
};

static int __init ext4_init(void)
{
	return register_filesystem(&ext4_fs_type);
}

module_init(ext4_init);
