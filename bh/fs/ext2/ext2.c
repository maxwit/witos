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

struct ext2_file_system {
	struct ext2_super_block sb;
	struct block_device *bdev;
	struct ext2_dir_entry_2 *root;
	struct ext2_group_desc *gdt;
};

static ssize_t ext2_read_block(struct ext2_file_system *fs, void *buff, int blk_no, size_t off, size_t size)
{
	struct block_device *bdev = fs->bdev;
	struct ext2_super_block *sb = &fs->sb;
	// struct disk_drive *drive = container_of(bdev, struct disk_drive, bdev);
	size_t buf_len = (off + size + SECT_SIZE - 1) & ~(SECT_SIZE - 1);
	char blk_buf[buf_len];
	int start_blk = blk_no << (sb->s_log_block_size + 1);
	// int cur_blk;
	struct bio *bio;

	bio = bio_alloc();
	if (!bio)
		return -ENOMEM;
	bio->bdev = bdev;
	bio->sect = start_blk;
	bio->size = buf_len;
	bio->data = blk_buf;
	submit_bio(READ, bio);
#if 0
	for (cur_blk = 0; cur_blk < buf_len / SECT_SIZE; cur_blk++) {
		// bdev->get_block(bdev, start_blk + cur_blk, blk_buf + cur_blk * SECT_SIZE);
		drive->get_block(drive, (start_blk + cur_blk) * SECT_SIZE, blk_buf + cur_blk * SECT_SIZE);
	}
#endif

	memcpy(buff, blk_buf + off, size);

	bio_free(bio);

	return size;
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

static size_t get_ind_block(struct ext2_file_system *fs,
			struct ext2_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	size_t block_size = 1 << (fs->sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	size_t i;

	start_block = start_block < 0 ? 0 : start_block;

	if(len <= 0)
		return 0;

	ext2_read_block(fs, buff, inode->i_block[EXT2_IND_BLOCK], 0, block_size);

	for(i = 0; i < len && i < index_per_block - start_block; i++)
		block_indexs[i] = buff[i + start_block];

	return i;
}

static size_t get_dind_block(struct ext2_file_system *fs,
			struct ext2_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	size_t block_size = 1 << (fs->sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	int i = 0, j, k;

	if(len <= 0)
		return 0;

	ext2_read_block(fs, buff, inode->i_block[EXT2_DIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	k = start_block % index_per_block;

	for(j = start_block / index_per_block; j < index_per_block; j++) {
		ext2_read_block(fs, dbuff, buff[j], 0, block_size);

		for(; i < len && k < index_per_block; i++, k++)
			block_indexs[i] = dbuff[k];

		if(i >= len)
			return i;

		k = 0;
	}

	return i;
}

static size_t get_tind_block(struct ext2_file_system *fs,
			struct ext2_inode *inode, ssize_t start_block, __le32 block_indexs[], size_t len)
{
	size_t block_size = 1 << (fs->sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	__le32 tbuff[index_per_block];
	int i = 0 /*? fixme*/, j, k, h;

	if(len <= 0)
		return 0;

	ext2_read_block(fs, buff, inode->i_block[EXT2_TIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	h = start_block % index_per_block;

	for(j = start_block / (index_per_block * index_per_block); j < index_per_block; j++) {
		ext2_read_block(fs, dbuff, buff[j], 0, block_size);

		for(k = start_block / index_per_block; k < index_per_block; k++) {
			ext2_read_block(fs, tbuff, dbuff[k], 0, block_size);

			for(; i < len && h < index_per_block; i++, h++)
				block_indexs[i] = tbuff[h];

			if(i >= len)
				return i;

			h = 0;
		}
	}

	return i;
}

static int get_block_indexs(struct ext2_file_system *fs,
			struct ext2_inode *inode,size_t start_block, __le32 block_indexs[], size_t len)
{
	size_t index_per_block = (1 << (fs->sb.s_log_block_size + 10)) / sizeof(__le32);
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
		i += get_ind_block(fs, inode, start_block - EXT2_IND_BLOCK,
							block_indexs + i, len - i);

		start_layer = -1;
	}

	if(i < len && (start_layer == 2 || start_layer == -1)) {
		i += get_dind_block(fs, inode, start_block - EXT2_IND_BLOCK - index_per_block
							, block_indexs + i, len - i);

		start_layer = -1;
	}

	if(i < len && (start_layer == 3 || start_layer == -1)) {
		i += get_tind_block(fs, inode, start_block - EXT2_IND_BLOCK - index_per_block
							- index_per_block * index_per_block, block_indexs + i, len - i);
	}

	return i;
}

static struct ext2_inode *ext2_read_inode(struct ext2_file_system *fs, int ino)
{
	struct ext2_inode *inode;
	struct ext2_super_block *sb = &fs->sb;
	int grp_no, ino_no, blk_no, count;
	struct ext2_group_desc *gde;

	ino--;

	count = (1 << (sb->s_log_block_size + 10)) / sb->s_inode_size;

	grp_no = ino / sb->s_inodes_per_group;
	gde = &fs->gdt[grp_no];

	ino_no = ino % sb->s_inodes_per_group;
	blk_no = ino_no / count;
	ino_no = ino_no % count;

	DPRINT("%s(%d): grp_no = %d, blk_no = %d, ino_no = %d, inode table = %d\n",
		__func__, ino + 1, grp_no, blk_no, ino_no, gde->bg_inode_table);

	inode = malloc(sb->s_inode_size);
	// if

	ext2_read_block(fs, inode, gde->bg_inode_table + blk_no, ino_no * sb->s_inode_size, sb->s_inode_size);

	DPRINT("%s(%d): inode size = %d, uid = %d, gid = %d, mode = 0x%x\n",
		__func__, ino + 1, inode->i_size, inode->i_uid, inode->i_gid, inode->i_mode);

	return inode;
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

static struct dentry *ext2_mount(struct file_system_type *fs_type, unsigned long flags, struct block_device *bdev)
{
	int blk_is;
	int gdt_num;
	struct dentry *root;
	struct inode *ino;
	struct ext2_file_system *ext2_fs = zalloc(sizeof(*ext2_fs));
	struct ext2_super_block *ext2_sb = &ext2_fs->sb;
	struct ext2_dir_entry_2 *ext2_root;
	struct ext2_group_desc *ext2_gdt;
	struct ext2_inode *ext2_ino;
	// struct disk_drive *drive = container_of(bdev, struct disk_drive, bdev);
	char buff[SECT_SIZE];
	struct bio *bio;

#if 0
	ret = drive->get_block(drive, 2 * SECT_SIZE, buff);
	if (ret < 0) {
		DPRINT("%s(): read dbr err\n", __func__);
		return NULL;
	}
#endif
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

	memcpy(ext2_sb, buff, sizeof(*ext2_sb));

	if (ext2_sb->s_magic != 0xEF53) {
		GEN_DBG("bad magic (0x%x)!\n", ext2_sb->s_magic);
		return NULL;
	}

	blk_is = (1 << (ext2_sb->s_log_block_size + 10)) / ext2_sb->s_inode_size;

	DPRINT("%s(): label = %s, log block size = %d, "
		"inode size = %d, block size = %d, inodes per block = %d\n",
		__func__, ext2_sb->s_volume_name, ext2_sb->s_log_block_size,
		ext2_sb->s_inode_size, (1 << (ext2_sb->s_log_block_size + 10)), blk_is);

	ext2_fs->bdev = bdev;

	gdt_num = (ext2_sb->s_blocks_count + ext2_sb->s_blocks_per_group - 1) / ext2_sb->s_blocks_per_group;;
	ext2_gdt = malloc(gdt_num * sizeof(struct ext2_group_desc));
	if (NULL == ext2_gdt)
		return NULL;

	ext2_read_block(ext2_fs, ext2_gdt, ext2_sb->s_first_data_block + 1, 0, gdt_num * sizeof(struct ext2_group_desc));

	DPRINT("%s(), block group[0 / %d]: free blocks= %d, free inodes = %d\n",
		__func__, gdt_num, ext2_gdt->bg_free_blocks_count, ext2_gdt->bg_free_inodes_count);

	ext2_fs->gdt  = ext2_gdt;

	ext2_root = malloc(sizeof(*ext2_root));
	// if ...

	ext2_root->inode = 2;

	ext2_fs->root = ext2_root;

	root = ext2_dentry_alloc();
	if (!root)
		return NULL;

	root->d_ext = ext2_root;
	///////////////

	ext2_ino = ext2_read_inode(ext2_fs, ext2_root->inode);
	// ...

	ino = root->inode;
	ino->mode = ~0; // ino->mode = ext2_ino->i_mode;
	ino->i_ext = ext2_ino;
	ino->i_fs = ext2_fs;

	return root;
}

#if 0
// fixme: free all resources
static int ext2_umount(const char *path)
{
	struct ext2_file_system *fs;

	fs = ext2_get_file_system(path);
	if (NULL == fs)
		return -ENOENT;

	// bdev_close(fs->bdev);

	// free fs-> ...

	free(fs);

	return 0;
}
#endif

static struct ext2_dir_entry_2 *ext2_real_lookup(struct inode *inode, const char *name)
{
	struct ext2_dir_entry_2 *d_match;
	struct ext2_dir_entry_2 *dentry;
	struct ext2_file_system *fs = inode->i_fs;
	struct ext2_inode *parent = inode->i_ext;
	char buff[parent->i_size];
	size_t len = 0;
	int blocks, i;
	size_t block_size = 1 << (fs->sb.s_log_block_size + 10);

	blocks = (parent->i_size + block_size - 1) / block_size;
	__le32 block_indexs[blocks];

	get_block_indexs(fs, parent, 0, block_indexs, blocks);

	for (i = 0; i < blocks; i++) {
		ext2_read_block(fs, buff, block_indexs[i], 0, block_size);

		dentry = (struct ext2_dir_entry_2 *)buff;

		while (dentry->rec_len > 0 && len < parent->i_size && len < (i + 1) * block_size) {
			dentry->name[dentry->name_len] = '\0';

			DPRINT("%s: inode = %d, dentry size = %d, name size = %d, block = %d\n",
				dentry->name, dentry->inode, dentry->rec_len, dentry->name_len, i);

			if (!strncmp(dentry->name, name, dentry->name_len))
				goto found_entry;

			dentry = (struct ext2_dir_entry_2 *)((char *)dentry + dentry->rec_len);
			len += dentry->rec_len;
		}
	}

	GEN_DBG("\"%s\" not found!\n", name);

	return NULL;

found_entry:
	d_match = malloc(sizeof(*d_match));
	*d_match = *dentry;

	return d_match;
}

// fixme: to parse path in syntax: "vfsmount:filename"
static inline int mnt_of_path(const char *path, char mnt[])
{
	return 0;
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
	struct ext2_inode *inode;
	struct ext2_file_system *fs = fp->de->inode->i_fs;
	struct ext2_dir_entry_2 *de;

	de = fp->de->d_ext;

	inode = ext2_read_inode(fs, de->inode);

	if (fp->pos == inode->i_size)
		return 0;

	block_size = 1 << (fs->sb.s_log_block_size + 10);
	char tmp_buff[block_size];

	real_size = min(size, inode->i_size - fp->pos);

	blocks = (real_size + (block_size - 1)) / block_size;
	__le32 block_indexs[blocks];

	get_block_indexs(fs, inode, fp->pos / block_size, block_indexs, blocks);

	offset = fp->pos % block_size;
	len = block_size - offset;

	for(i = 0; i < blocks; i++) {
		if(i == blocks - 1)
			len = real_size % block_size == 0 ? block_size : (real_size % block_size);

		ext2_read_block(fs, tmp_buff, block_indexs[i], offset, len);

		memcpy(buff, tmp_buff, len);
		buff += len;
		fp->pos += len;
		offset = 0;
		len = block_size;
	}

	return real_size;
}

static ssize_t ext2_write(struct file *fp, const void *buff, size_t size, loff_t *off)
{
	return 0;
}

static struct dentry *ext2_lookup(struct inode *parent, const char *name)
{
	struct inode *ino;
	struct dentry *de;
	struct ext2_dir_entry_2 *ext2_de;
	struct ext2_inode *ext2_ino;

	de = ext2_dentry_alloc();
	if (!de)
		return NULL;
	ino = de->inode;

	ext2_de = ext2_real_lookup(parent, name);
	if (!ext2_de)
		return NULL;

	de->d_ext = ext2_de;

	ext2_ino = ext2_read_inode(parent->i_fs, ext2_de->inode);
	// ...
	ino->mode = ~0; // ino->mode = ext2_ino->i_mode;
	ino->i_ext = ext2_ino;
	ino->i_fs = parent->i_fs;

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
	.fops	= &ext2_fops,
	.mount  = ext2_mount,
	.lookup = ext2_lookup,
};

static int __INIT__ ext2_init(void)
{
	return file_system_type_register(&ext2_fs_type);
}

SUBSYS_INIT(ext2_init);
