#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <assert.h>
#include <fs/fs.h>

static struct file_system_type *fs_type_list;
static DECL_INIT_LIST(g_mount_list);

int file_system_type_register(struct file_system_type *fstype)
{
	struct file_system_type **p;

	for (p = &fs_type_list; *p; p = &(*p)->next) {
		if (!strcmp((*p)->name, fstype->name))
			return -EBUSY;
	}

	fstype->next = NULL;
	*p = fstype;

	return 0;
}

struct file_system_type *file_system_type_get(const char *name)
{
	struct file_system_type *p;

	for (p = fs_type_list; p; p = p->next) {
		if (!strcmp(p->name, name))
			return p;
	}

	return NULL;
}

int sys_mount(const char *dev_name, const char *path,
		const char *type, unsigned long flags)
{
	int ret;
	static bool root_mounted = false;
	struct file_system_type *fstype;
	struct vfsmount *mnt;
	struct dentry *root;
	struct nameidata nd;

	if ((flags & MS_ROOT) && root_mounted == true)
		return -EBUSY;

	if (!(flags & MS_NODEV)) {
		list_for_each_entry(mnt, &g_mount_list, mnt_hash) {
			if (!strcmp(mnt->dev_name, dev_name)) {
				GEN_DBG("device \"%s\" already mounted!\n", dev_name);
				return -EBUSY;
			}
		}
	}

	if (!(flags & MS_ROOT)) {
		ret = path_walk(path, &nd); // fixme: directory only!
		if (ret < 0) {
			GEN_DBG("\"%s\" does NOT exist!\n");
			return ret;
		}
	}

	fstype = file_system_type_get(type);
	if (NULL == fstype) {
		DPRINT("fail to find file system type %s!\n", type);
		return -ENOENT;
	}

	root = fstype->mount(fstype, flags, dev_name);
	if (!root) {
		DPRINT("fail to mount %s!\n", dev_name);
		ret = -EIO; // fixme!
		goto L1;
	}

	mnt = zalloc(sizeof(*mnt));
	if (NULL == mnt) {
		ret = -ENOMEM;
		goto L2;
	}

	mnt->fstype   = fstype;
	mnt->dev_name = dev_name;
	mnt->root     = root;

	if (flags & MS_ROOT) {
		struct path sysroot = {.dentry = root, .mnt = mnt};

		mnt->mountpoint = NULL; // fixme
		mnt->mnt_parent = NULL;
		set_fs_root(&sysroot);
		set_fs_pwd(&sysroot); // now?
	} else {
		mnt->mountpoint = nd.path.dentry;
		mnt->mnt_parent = nd.path.mnt;
	}

	list_add_tail(&mnt->mnt_hash, &g_mount_list);

	return 0;
L2:
	//
L1:
	return ret;
}

int GAPI mount(const char *source, const char *target,
			const char *type, unsigned long flags)
{
	return sys_mount(source, target, type, flags);
}

int GAPI umount(const char *mnt)
{
	return 0;
}

struct vfsmount *lookup_mnt(struct path *path)
{
	struct vfsmount *mnt;

	list_for_each_entry(mnt, &g_mount_list, mnt_hash) {
		if (mnt->mountpoint == path->dentry)
			return mnt;
	}

	return NULL;
}

// TODO: move to app layer
int list_mount()
{
	int count = 0;
	struct vfsmount *mnt;

	printf("NO. device   mnt  type\n"
		"-------------------------\n");
	list_for_each_entry(mnt, &g_mount_list, mnt_hash) {
		count++;
		printf("(%d) %-8s %-4s %s\n", count,
			mnt->dev_name ? mnt->dev_name : "none",
			mnt->mountpoint ? mnt->mountpoint->d_name.name : "/", // fixme
			mnt->fstype->name);
	}

	return count;
}
