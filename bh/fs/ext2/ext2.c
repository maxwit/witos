#include <stdio.h>
#include <init.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <block.h>
#include <fs/fs.h>
#include <fs/ext2.h>

#define MAX_MNT_LEN 256
#define SECT_SIZE   (1 << 9)

struct ext2_sb_info {
	struct ext2_super_block e2_sb;
	struct ext2_group_desc *gdt;
};

static ssize_t ext2_read_block(struct super_block *sb, void *buff, int blk_no, size_t off, size_t size)
{
	ssize_t ret, start_blk;
	struct bio *bio;
	struct ext2_sb_info *e2_sbi = sb->s_ext;
	struct ext2_super_block *e2_sb;

	e2_sb = &e2_sbi->e2_sb;
	start_blk = blk_no << (e2_sb->s_log_block_size + 1);

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
	if(start_block < EXT2_IND_BLOCK)
		return 0;
	else if(start_block < EXT2_IND_BLOCK + index_per_block)
		return 1;
	else if(start_block < EXT2_IND_BLOCK + index_per_block +
			index_per_block * index_per_block)
		return 2;
	else
		return 3;
}

static size_t get_ind_block(struct super_block *sb,
			struct ext2_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	struct ext2_sb_info *e2_sbi = sb->s_ext;
	size_t block_size = 1 << (e2_sbi->e2_sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	size_t i;

	start_block = start_block < 0 ? 0 : start_block;

	if(len <= 0)
		return 0;

	ext2_read_block(sb, buff, inode->i_block[EXT2_IND_BLOCK], 0, block_size);

	for(i = 0; i < len && i < index_per_block - start_block; i++)
		block_indexs[i] = buff[i + start_block];

	return i;
}

static size_t get_dind_block(struct super_block *sb,
			struct ext2_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	struct ext2_sb_info *e2_sbi = sb->s_ext;
	size_t block_size = 1 << (e2_sbi->e2_sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	int i = 0, j, k;

	if(len <= 0)
		return 0;

	ext2_read_block(sb, buff, inode->i_block[EXT2_DIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	k = start_block % index_per_block;

	for(j = start_block / index_per_block; j < index_per_block; j++) {
		ext2_read_block(sb, dbuff, buff[j], 0, block_size);

		while (i < len && k < index_per_block)
			block_indexs[i++] = dbuff[k++];

		if(i >= len)
			return i;

		k = 0;
	}

	return i;
}

static size_t get_tind_block(struct super_block *sb,
			struct ext2_inode *inode, ssize_t start_block, __le32 block_indexs[], size_t len)
{
	struct ext2_sb_info *e2_sbi = sb->s_ext;
	size_t block_size = 1 << (e2_sbi->e2_sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	__le32 tbuff[index_per_block];
	int i = 0 /*? fixme*/, j, k, h;

	if(len <= 0)
		return 0;

	ext2_read_block(sb, buff, inode->i_block[EXT2_TIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	h = start_block % index_per_block;

	for(j = start_block / (index_per_block * index_per_block); j < index_per_block; j++) {
		ext2_read_block(sb, dbuff, buff[j], 0, block_size);

		for(k = start_block / index_per_block; k < index_per_block; k++) {
			ext2_read_block(sb, tbuff, dbuff[k], 0, block_size);

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
			struct ext2_inode *inode,size_t start_block, __le32 block_indexs[], size_t len)
{
	struct ext2_sb_info *e2_sbi = sb->s_ext;
	size_t index_per_block = (1 << (e2_sbi->e2_sb.s_log_block_size + 10)) / sizeof(__le32);
	size_t start_layer;
	size_t i = 0;

	if (NULL == inode)
		return -1;

	if(len <= 0)
		return 0;

	start_layer = get_start_layer(start_block, index_per_block);

	if (0 == start_layer) {
		for(i = 0; i < len && i < EXT2_IND_BLOCK - start_block; i++)
			block_indexs[i] = inode->i_block[i + start_block];

		start_layer = -1;
	}

	if(i < len && (start_layer == 1 || start_layer == -1)) {
		i += get_ind_block(sb, inode, start_block - EXT2_IND_BLOCK,
							block_indexs + i, len - i);

		start_layer = -1;
	}

	if(i < len && (start_layer == 2 || start_layer == -1)) {
		i += get_dind_block(sb, inode, start_block - EXT2_IND_BLOCK - index_per_block
							, block_indexs + i, len - i);

		start_layer = -1;
	}

	if(i < len && (start_layer == 3 || start_layer == -1)) {
		i += get_tind_block(sb, inode, start_block - EXT2_IND_BLOCK - index_per_block
							- index_per_block * index_per_block, block_indexs + i, len - i);
	}

	return i;
}

static struct ext2_inode *ext2_read_inode(struct super_block *sb, int ino)
{
	int grp_no, ino_no, blk_no, count;
	struct ext2_sb_info *e2_sbi = sb->s_ext;
	struct ext2_inode *e2_in;
	struct ext2_super_block *e2_sb;
	struct ext2_group_desc *gde;

	ino--;
	e2_sb = &e2_sbi->e2_sb;

	count = (1 << (e2_sb->s_log_block_size + 10)) / e2_sb->s_inode_size;

	grp_no = ino / e2_sb->s_inodes_per_group;
	gde = &e2_sbi->gdt[grp_no];

	ino_no = ino % e2_sb->s_inodes_per_group;
	blk_no = ino_no / count;
	ino_no = ino_no % count;

	DPRINT("%s(%d): grp_no = %d, blk_no = %d, ino_no = %d, inode table = %d\n",
		__func__, ino + 1, grp_no, blk_no, ino_no, gde->bg_inode_table);

	e2_in = malloc(e2_sb->s_inode_size);
	// if

	ext2_read_block(sb, e2_in, gde->bg_inode_table + blk_no, ino_no * e2_sb->s_inode_size, e2_sb->s_inode_size);

	DPRINT("%s(%d): inode size = %d, uid = %d, gid = %d, mode = 0x%x\n",
		__func__, ino + 1, e2_in->i_size, e2_in->i_uid, e2_in->i_gid, e2_in->i_mode);

	return e2_in;
}

static struct dentry *ext2_dentry_alloc()
{
	struct dentry *de;
	struct inode *ino;

	de = zalloc(sizeof(*de));
	if (!de)
		return NULL;

	ino = zalloc(sizeof(*ino));
	if (!ino)
		return NULL;

	de->inode = ino;

	return de;
}

bool ext2_check_fstype(void *buff, size_t size)
{
	struct ext2_super_block *e2_sb = buff;

	if (size >= sizeof(struct ext2_super_block) && 0xEF53 == e2_sb->s_magic)
		return true;

	return false;
}

static struct dentry *ext2_mount(struct file_system_type *fs_type, unsigned long flags, struct block_device *bdev)
{
	int blk_is;
	int gdt_num;
	char buff[SECT_SIZE];
	struct dentry *root;
	struct inode *ino;
	struct super_block *sb;
	struct ext2_sb_info *e2_sbi;
	struct ext2_super_block *e2_sb;
	struct ext2_dir_entry_2 *e2_root;
	struct ext2_group_desc *gdt;
	struct ext2_inode *e2_in;
	struct bio *bio;

	bio = bio_alloc();
	if (!bio)
		return NULL;
	bio->bdev = bdev;
	bio->sect = 2;
	bio->size = SECT_SIZE;
	bio->data = buff;
	submit_bio(READ, bio);
	// TODO: check flags here
	bio_free(bio);

	if (ext2_check_fstype(buff, SECT_SIZE) == true) {
		GEN_DBG("Invalid EXT2 magic number!\n"); // ((struct ext2_super_block *)buff)->s_magic);
		return NULL;
	}

	e2_sbi = zalloc(sizeof(*e2_sbi));
	if (!e2_sbi) {
		// ...
		return NULL;
	}

	e2_sb = &e2_sbi->e2_sb;
	memcpy(e2_sb, buff, sizeof(*e2_sb));

	sb = zalloc(sizeof(*sb));
	if (!sb) {
		// ...
		return NULL;
	}

	sb->s_ext  = e2_sbi;
	sb->s_bdev = bdev;

	blk_is = (1 << (e2_sb->s_log_block_size + 10)) / e2_sb->s_inode_size;

	DPRINT("%s(): label = %s, log block size = %d, "
		"inode size = %d, block size = %d, inodes per block = %d\n",
		__func__, e2_sb->s_volume_name, e2_sb->s_log_block_size,
		e2_sb->s_inode_size, (1 << (e2_sb->s_log_block_size + 10)), blk_is);

	gdt_num = (e2_sb->s_blocks_count + e2_sb->s_blocks_per_group - 1) / e2_sb->s_blocks_per_group;;
	gdt = malloc(gdt_num * sizeof(struct ext2_group_desc));
	if (NULL == gdt)
		return NULL;

	ext2_read_block(sb, gdt, e2_sb->s_first_data_block + 1, 0, gdt_num * sizeof(struct ext2_group_desc));

	DPRINT("%s(), block group[0 / %d]: free blocks= %d, free inodes = %d\n",
		__func__, gdt_num, gdt->bg_free_blocks_count, gdt->bg_free_inodes_count);

	e2_sbi->gdt = gdt;

	e2_root = malloc(sizeof(*e2_root));
	// if ...

	e2_root->inode = 2;
	// TODO: fill other fields

	root = ext2_dentry_alloc();
	if (!root)
		return NULL;

	root->d_ext = e2_root;
	root->d_sb  = sb;
	// ...

	sb->s_root = root;

	e2_in = ext2_read_inode(sb, e2_root->inode);
	// ...

	ino = root->inode;
	ino->mode  = e2_in->i_mode; // fixme
	ino->i_ext = e2_in;
	ino->i_sb  = sb;

	return root;
}

// fixme
static void ext2_umount(struct super_block *sb)
{
}

static struct ext2_dir_entry_2 *ext2_real_lookup(struct inode *inode, struct nameidata *nd)
{
	struct super_block *sb = inode->i_sb;
	struct ext2_sb_info *e2_sbi;
	struct ext2_dir_entry_2 *e2_de, *e2_de_iter;
	struct ext2_inode *parent = inode->i_ext;
	char buff[parent->i_size];
	size_t len = 0;
	int blocks, i;
	size_t block_size;
	struct qstr *unit = nd->unit;

	e2_sbi = sb->s_ext;
	block_size = 1024 << e2_sbi->e2_sb.s_log_block_size;

	blocks = (parent->i_size + block_size - 1) / block_size;
	__le32 block_indexs[blocks];

	get_block_indexs(sb, parent, 0, block_indexs, blocks);

	for (i = 0; i < blocks; i++) {
		ext2_read_block(sb, buff, block_indexs[i], 0, block_size);

		e2_de = (struct ext2_dir_entry_2 *)buff;

		while (e2_de->rec_len > 0 && len < parent->i_size && len < (i + 1) * block_size) {
			e2_de->name[e2_de->name_len] = '\0';

			DPRINT("%s: inode = %d, e2_de size = %d, name size = %d, block = %d\n",
				e2_de->name, e2_de->inode, e2_de->rec_len, e2_de->name_len, i);

			if (unit->len == e2_de->name_len && \
				!strncmp(e2_de->name, unit->name, e2_de->name_len))
				goto found_entry;

			e2_de = (struct ext2_dir_entry_2 *)((char *)e2_de + e2_de->rec_len);
			len += e2_de->rec_len;
		}
	}

	GEN_DBG("\"%s\" not found!\n", unit->name);

	return NULL;

found_entry:
	e2_de_iter = malloc(sizeof(*e2_de_iter));
	*e2_de_iter = *e2_de;

	return e2_de_iter;
}

static int ext2_open(struct file *fp, struct inode *inode)
{
	return 0;
}

static int ext2_close(struct file *fp)
{
	return 0;
}

static ssize_t ext2_read(struct file *fp, void *buff, size_t size, loff_t *off)
{
	ssize_t i, len;
	size_t blocks;
	size_t offset, real_size, block_size;
	struct dentry *de = fp->f_dentry;
	struct super_block *sb;
	struct ext2_inode *e2_in;
	struct ext2_sb_info *e2_sbi;
	struct ext2_dir_entry_2 *e2_de;

	sb     = de->d_sb;
	e2_de  = de->d_ext;
	e2_sbi = sb->s_ext;

	e2_in = ext2_read_inode(sb, e2_de->inode);
	if (!e2_in)
		return -ENOENT;

	if (fp->f_pos == e2_in->i_size)
		return 0;

	block_size = 1 << (e2_sbi->e2_sb.s_log_block_size + 10);
	char tmp_buff[block_size];

	real_size = min(size, e2_in->i_size - fp->f_pos);

	blocks = (real_size + (block_size - 1)) / block_size;
	__le32 block_indexs[blocks];

	get_block_indexs(sb, e2_in, fp->f_pos / block_size, block_indexs, blocks);

	offset = fp->f_pos % block_size;
	len = block_size - offset;

	for(i = 0; i < blocks; i++) {
		if(i == blocks - 1)
			len = real_size % block_size == 0 ? block_size : (real_size % block_size);

		ext2_read_block(sb, tmp_buff, block_indexs[i], offset, len);

		memcpy(buff, tmp_buff, len);
		buff += len;
		fp->f_pos += len;
		offset = 0;
		len = block_size;
	}

	return real_size;
}

static ssize_t ext2_write(struct file *fp, const void *buff, size_t size, loff_t *off)
{
	return 0;
}

static struct dentry *ext2_lookup(struct inode *parent, struct nameidata *nd)
{
	struct inode *in;
	struct dentry *de;
	struct ext2_dir_entry_2 *e2_de;
	struct ext2_inode *e2_in;

	e2_de = ext2_real_lookup(parent, nd);
	if (!e2_de)
		return NULL;

	de = ext2_dentry_alloc();
	if (!de)
		return NULL;

	de->d_ext = e2_de;
	de->d_sb  = parent->i_sb;

	e2_in = ext2_read_inode(parent->i_sb, e2_de->inode);
	if (!e2_in)
		return NULL;

	in = de->inode;
	in->mode  = e2_in->i_mode;
	in->i_ext = e2_in;
	in->i_sb  = parent->i_sb;

	return de;
}

static const struct file_operations ext2_fops = {
	.open  = ext2_open,
	.close = ext2_close,
	.read  = ext2_read,
	.write = ext2_write,
};

static struct file_system_type ext2_fs_type = {
	.name   = "ext2",
	.mount  = ext2_mount,
	.umount = ext2_umount,
	// to be removed:
	.fops	= &ext2_fops,
	.lookup = ext2_lookup,
};

static int __INIT__ ext2_init(void)
{
	GEN_DBG("\n");
	return file_system_type_register(&ext2_fs_type);
}

module_init(ext2_init);
