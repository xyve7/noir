#pragma once

#include <kernel.h>
#include <stddef.h>
#include <stdint.h>

typedef uint64_t vtime;
typedef uint8_t vflags;

#define VFS_NAME_MAX 256
#define VFS_PATH_MAX 4096

#define VFS_READ 0b00000001
#define VFS_WRITE 0b00000010
#define VFS_EXEC 0b00000100

typedef enum {
    VFS_FILE,
    VFS_DIR,
    VFS_DEV,
    VFS_MOUNT
} vtype;

typedef struct vnode vnode;
typedef struct vmount vmount;

typedef struct {
    // Univeral parameters
    vtype type;
    vflags flags;

    vtime ctime;
    vtime mtime;
    vtime atime;

    size_t size;
    size_t count; // This only applies to directories
} vinfo;

typedef struct {
    error (*mount)(vnode *device, vmount **mount);
} vfs_ops;

typedef struct {
    char name[16];
    vfs_ops ops;
} vfs;

typedef struct vmount {
    vfs *fs;
    vnode *root;
} vmount;

typedef struct {
    error (*open)(vnode *node, vflags flags);
    error (*read)(vnode *node, void *buffer, size_t offset, size_t size);
    error (*write)(vnode *node, const void *buffer, size_t offset, size_t size);
    error (*close)(vnode *node);

    error (*info)(vnode *node, vinfo *info);
    error (*find)(vnode *parent, const char *name, vnode **found);
    error (*entry)(vnode *parent, size_t index, vnode **found);

    error (*map)(vnode *node, size_t offset, size_t size, void **where);

    error (*create)(vnode *parent, const char *name, vtype type, vflags flags);
    error (*rename)(vnode *node, const char *new_name);
    error (*delete)(vnode *node);
} vnode_ops;

typedef struct vnode {
    // File name
    char name[VFS_NAME_MAX];
    // Supported operations
    vnode_ops ops;
} vnode;

error vfs_init();

error vfs_open(const char *path, vflags flags, vnode **result);
error vfs_read(vnode *node, void *buffer, size_t offset, size_t size);
error vfs_write(vnode *node, const void *buffer, size_t offset, size_t size);
error vfs_close(vnode *node);

error vfs_info(const char *path, vinfo *info);
error vfs_entry(const char *path, size_t index, vnode **found);

error vfs_map(vnode *node, size_t offset, size_t size, void **where);

error vfs_create(const char *path, vtype type, vflags flags);
error vfs_rename(const char *old_name, const char *new_name);
error vfs_delete(vnode *node);

error vfs_register(vfs *fs);
error vfs_mount_raw(vnode *device, const char *where, const char *fs);
error vfs_mount(const char *device, const char *where, const char *fs);
