#include <stdio.h>
#include <init.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <block.h>
#include <dirent.h>
#include <fs/fs.h>

#define RAM_BLK_SIZE   (1 << 9)

struct ram_super_block {
	struct list_node *r_list;
};

struct ram_inode {
	struct inode vfs_inode;
	void *data;
};

// fixme
static DECL_INIT_LIST(g_root_dir);

static struct dentry *ram_lookup(struct inode *parent, struct nameidata *nd);
static int ram_open(struct file *fp, struct inode *inode);
static int ram_close(struct file *fp);
static ssize_t ram_read(struct file *fp, void *buff, size_t size, loff_t *off);
static ssize_t ram_write(struct file *fp, const void *buff, size_t size, loff_t *off);
static int ram_readdir(struct file *, struct linux_dirent *);
static int ram_mkdir(struct inode *, struct dentry *, int);

static const struct inode_operations ram_reg_inode_operations = {
};

static const struct file_operations ram_reg_file_operations = {
	.open  = ram_open,
	.close = ram_close,
	.read  = ram_read,
	.write = ram_write,
};

static const struct inode_operations ram_dir_inode_operations = {
	.lookup = ram_lookup,
	.mkdir  = ram_mkdir,
};

static const struct file_operations ram_dir_file_operations = {
	.readdir = ram_readdir,
};

static struct inode *ram_alloc_inode(struct super_block *sb)
{
	struct ram_inode *rin;

	rin = zalloc(sizeof(*rin));
	if (!rin)
		return NULL;

	rin->vfs_inode.i_sb = sb; // fixme

	return &rin->vfs_inode;
}

static ssize_t ram_read_block(struct super_block *sb, void *buff, int blk_no, size_t off, size_t size)
{
	ssize_t ret, start_blk;
	struct bio *bio;
	struct ram_super_block *ram_sb = sb->s_fs_info;
	struct ram_super_block *e2_sb;

	e2_sb = &ram_sb->e2_sb;
	start_blk = blk_no << (e2_sb->s_log_block_size + 1);

	bio = bio_alloc();
	if (!bio)
		return -ENOMEM;

	bio->size = (off + size + RAM_BLK_SIZE - 1) & ~(RAM_BLK_SIZE - 1);
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
			struct ram_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	struct ram_super_block *ram_sb = sb->s_fs_info;
	size_t block_size = 1 << (ram_sb->e2_sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	size_t i;

	start_block = start_block < 0 ? 0 : start_block;

	if(len <= 0)
		return 0;

	ram_read_block(sb, buff, inode->i_block[EXT2_IND_BLOCK], 0, block_size);

	for(i = 0; i < len && i < index_per_block - start_block; i++)
		block_indexs[i] = buff[i + start_block];

	return i;
}

static size_t get_dind_block(struct super_block *sb,
			struct ram_inode *inode,ssize_t start_block, __le32 block_indexs[], size_t len)
{
	struct ram_super_block *ram_sb = sb->s_fs_info;
	size_t block_size = 1 << (ram_sb->e2_sb.s_log_block_size + 10);
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	int i = 0, j, k;

	if(len <= 0)
		return 0;

	ram_read_block(sb, buff, inode->i_block[EXT2_DIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	k = start_block % index_per_block;

	for(j = start_block / index_per_block; j < index_per_block; j++) {
		ram_read_block(sb, dbuff, buff[j], 0, block_size);

		while (i < len && k < index_per_block)
			block_indexs[i++] = dbuff[k++];

		if(i >= len)
			return i;

		k = 0;
	}

	return i;
}

static size_t get_tind_block(struct super_block *sb,
			struct ram_inode *inode, ssize_t start_block, __le32 block_indexs[], size_t len)
{
	size_t block_size = sb->s_blksize;
	size_t index_per_block = block_size / sizeof(__le32);
	__le32 buff[index_per_block];
	__le32 dbuff[index_per_block];
	__le32 tbuff[index_per_block];
	int i = 0 /*? fixme*/, j, k, h;

	if(len <= 0)
		return 0;

	ram_read_block(sb, buff, inode->i_block[EXT2_TIND_BLOCK], 0, block_size);

	start_block = start_block < 0 ? 0 : start_block;

	h = start_block % index_per_block;

	for(j = start_block / (index_per_block * index_per_block); j < index_per_block; j++) {
		ram_read_block(sb, dbuff, buff[j], 0, block_size);

		for(k = start_block / index_per_block; k < index_per_block; k++) {
			ram_read_block(sb, tbuff, dbuff[k], 0, block_size);

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
			struct ram_inode *inode,size_t start_block, __le32 block_indexs[], size_t len)
{
	struct ram_super_block *ram_sb = sb->s_fs_info;
	size_t index_per_block = (1 << (ram_sb->e2_sb.s_log_block_size + 10)) / sizeof(__le32);
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

static struct ram_inode *ram_get_inode(struct super_block *sb, int ino)
{
	int grp_no, ino_no, blk_no, count;
	struct ram_super_block *ram_sb = sb->s_fs_info;
	struct ram_inode *e2_in;
	struct ram_super_block *e2_sb;
	struct ram_group_desc *gde;

	ino--;
	e2_sb = &ram_sb->e2_sb;

	count = (1 << (e2_sb->s_log_block_size + 10)) / e2_sb->s_inode_size;

	grp_no = ino / e2_sb->s_inodes_per_group;
	gde = &ram_sb->gdt[grp_no];

	ino_no = ino % e2_sb->s_inodes_per_group;
	blk_no = ino_no / count;
	ino_no = ino_no % count;

	DPRINT("%s(%d): grp_no = %d, blk_no = %d, ino_no = %d, inode table = %d\n",
		__func__, ino + 1, grp_no, blk_no, ino_no, gde->bg_inode_table);

	e2_in = malloc(e2_sb->s_inode_size);
	// if

	ram_read_block(sb, e2_in, gde->bg_inode_table + blk_no, ino_no * e2_sb->s_inode_size, e2_sb->s_inode_size);

	DPRINT("%s(%d): inode size = %d, uid = %d, gid = %d, mode = 0x%x\n",
		__func__, ino + 1, e2_in->i_size, e2_in->i_uid, e2_in->i_gid, e2_in->i_mode);

	return e2_in;
}

bool ram_check_fstype(void *buff, size_t size)
{
	struct ram_super_block *e2_sb = buff;

	if (size >= sizeof(struct ram_super_block) && 0xEF53 == e2_sb->s_magic)
		return true;

	return false;
}

struct inode *ram_iget(struct super_block *sb, unsigned long ino)
{
	struct inode *inode;
	struct ram_inode *e2_ini;

	inode = ram_alloc_inode(sb);
	if (!ino) {
		// ...
		return NULL;
	}

	inode->i_ino  = ino;

	e2_ini = EXT2_I(inode);

	if (S_ISREG(inode->i_mode)) {
		inode->i_op = &ram_reg_inode_operations;
		inode->i_fop = &ram_reg_file_operations;
	} else if (S_ISDIR(inode->i_mode)) {
		inode->i_op = &ram_dir_inode_operations;
		inode->i_fop = &ram_dir_file_operations;
	} else {
		BUG();
	}

	return inode;
}

static int ram_fill_super(struct super_block *sb)
{
	int ret;
	struct ram_super_block *ram_sb;

	ram_sb = zalloc(sizeof(*ram_sb));
	if (!ram_sb) {
		ret = -ENOMEM;
		goto L1;
	}

	sb->s_fs_info = ram_sb;
	sb->s_blksize = RAM_BLK_SIZE;

	return 0;
L1:
	return ret;
}

static struct dentry *ram_mount(struct file_system_type *fs_type, unsigned long flags, const char *bdev_name)
{
	int ret;
	struct dentry *root;
	struct inode *in;
	struct super_block *sb;

	sb = sget(fs_type, NULL);
	if (!sb)
		return NULL;

	ret = ram_fill_super(sb);
	if (ret < 0) {
		// ...
		return NULL;
	}

	in = ram_iget(sb, 1);
	if (!in) {
		// ...
		return NULL;
	}

	root = d_alloc_root(in);
	if (!root)
		return NULL;

	sb->s_root = root;

	return root;
}

// fixme
static void ram_umount(struct super_block *sb)
{
}

static int ram_open(struct file *fp, struct inode *inode)
{
	struct ram_inode *rin = container_of(inode, struct ram_inode, vfs_inode);

	fp->private_data = rin->data;

	return 0;
}

static int ram_close(struct file *fp)
{
	return 0;
}

static ssize_t ram_read(struct file *fp, void *buff, size_t size, loff_t *off)
{
	return 0;
}

static ssize_t ram_write(struct file *fp, const void *buff, size_t size, loff_t *off)
{
	return 0;
}

static struct dentry *ram_lookup(struct inode *parent, struct nameidata *nd)
{
	unsigned long ino;
	struct inode *inode;
	struct dentry *dir, *child;
	struct ram_inode *rin;
	struct list_node *iter;

	rin = container_of(parent, struct inode, vfs_inode);
	dir = rin->data;

	list_for_each(iter, &dir->d_subdirs) {
		child = container_of(iter, struct dentry, d_child);
		if (!strcmp(child->d_name.name, nd->unit->name))
			return child;
	}

	return NULL;
}

static int ram_readdir(struct file *fp, struct linux_dirent *dirent)
{
	struct inode *inode;
	struct ram_inode *rin;
	struct linux_dirent *rde;

	inode = fp->f_dentry->d_inode;
	if (fp->f_pos >= inode->i_size)
		return 0;

	rin = container_of(inode, struct ram_inode, vfs_node);
	rde = rin->data + fp->f_pos;
	memcpy(dirent, rde, rde->d_reclen);

	fp->f_pos += rde->d_reclen;
	return rde->d_reclen;
}

static int ram_mkdir(struct inode *parent, struct dentry *de, int mode)
{
	struct inode *in;

	in = ram_alloc_inode(parent->i_sb);
	//
	in->i_ino = random(); //
	in->i_mode = mode;

	in->i_op = &ram_dir_inode_operations;
	in->i_fop = &ram_dir_file_operations;

	de->d_inode = in;

	return 0;
}

static struct file_system_type ram_fs_type = {
	.name   = "tmpfs",
	.mount  = ram_mount,
	.umount = ram_umount,
};

static int __INIT__ ram_init(void)
{
	return file_system_type_register(&ram_fs_type);
}

module_init(ram_init);
