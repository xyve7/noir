#include <fs/tarfs.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <mm/heap.h>

error tarfs_mount(vnode *device, vmount **mount);
error tarfs_open(vnode *node, vflags flags);
error tarfs_read(vnode *node, void *buffer, size_t offset, size_t size);
error tarfs_write(vnode *node, const void *buffer, size_t offset, size_t size);
error tarfs_close(vnode *node);
error tarfs_info(vnode *node, vinfo *info);
error tarfs_find(vnode *parent, const char *name, vnode **found);
error tarfs_entry(vnode *parent, size_t index, vnode **found);
error tarfs_create(vnode *parent, const char *name, vtype type, vflags flags);
error tarfs_rename(vnode *node, const char *new_name);
error tarfs_delete(vnode *node);

typedef struct {
    vnode node;
    vnode *device;
    size_t offset;
    bool is_root;
} tarfs_node;

#define TAR_REG '0'
#define TAR_DIR '5'
#define TAR_MNT '8' // This is a custom extension

typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char reserved[12];
} posix_header;

vfs_ops tarfs_vfs_ops = {
    .mount = tarfs_mount
};

vfs *tarfs_vfs = nullptr;

vnode_ops tarfs_ops = {
    .open = tarfs_open,
    .read = tarfs_read,
    .write = tarfs_write,
    .close = tarfs_close,
    .info = tarfs_info,
    .find = tarfs_find,
    .entry = tarfs_entry,
    .create = tarfs_create,
    .rename = tarfs_rename,
    .delete = tarfs_delete,
};
int otob(char *str) {
    size_t size = strlen(str);
    int n = 0;
    char *c = str;
    while (size-- > 0) {
        n *= 8;
        n += *c - '0';
        c++;
    }
    return n;
}
void tarfs_init() {
    // Create VFS
    tarfs_vfs = kmalloc(sizeof(vfs));
    strcpy(tarfs_vfs->name, "tarfs");
    tarfs_vfs->ops = tarfs_vfs_ops;

    vfs_register(tarfs_vfs);

    LOG("TAR Filesystem Initialized");
}
error tarfs_mount(vnode *device, vmount **mount) {
    // We create the root
    tarfs_node *root = kmalloc(sizeof(tarfs_node));
    memset(root, 0, sizeof(*root));
    root->node.ops = tarfs_ops;
    root->device = device;
    root->offset = 0;
    root->is_root = true;

    // Create mountpoint
    *mount = kmalloc(sizeof(vmount));
    (*mount)->root = (vnode *)root;
    (*mount)->fs = tarfs_vfs;

    return OK;
}
error tarfs_open(vnode *node, vflags flags) {
    return OK;
}
error tarfs_read(vnode *node, void *buffer, size_t offset, size_t size) {
    tarfs_node *n = (tarfs_node *)node;
    vfs_read(n->device, buffer, sizeof(posix_header) + n->offset + offset, size);
    return OK;
}
error tarfs_write(vnode *node, const void *buffer, size_t offset, size_t size) {
    return OK;
}
error tarfs_close(vnode *node) {
    return OK;
}
error tarfs_info(vnode *node, vinfo *info) {
    tarfs_node *n = (tarfs_node *)node;

    // We read the header
    posix_header *header = kmalloc(sizeof(posix_header));
    vfs_read(n->device, header, n->offset, sizeof(posix_header));

    // Get the type
    switch (header->typeflag) {
    case TAR_REG:
        info->type = VFS_FILE;
        break;
    case TAR_DIR:
        info->type = VFS_DIR;
        break;
    case TAR_MNT:
        info->type = VFS_MOUNT;
        break;
    }

    info->size = otob(header->size);

    size_t count = 0;
    // Get count
    if (header->typeflag == TAR_DIR || n->is_root) {
        size_t offset = n->offset;

        // We read the header
        vfs_read(n->device, header, offset, sizeof(posix_header));

        // Offset 0 means root
        size_t parent_slash = offset == 0 ? 0 : strcnt(header->name, '/');
        while (memcmp(header->magic, "ustar", 5) == 0) {
            // Get information
            size_t s = otob(header->size);
            size_t child_slash = strcnt(header->name, '/');

            // If its regular it wont have an ending slash
            // So we add it
            if (header->typeflag == TAR_REG) {
                child_slash++;
            }

            // If we have one more slash than the root
            if (parent_slash + 1 == child_slash) {
                count++;
            }

            // Skip past the header and data
            offset += (((s + 511) / 512) + 1) * 512;

            // We read the next header
            vfs_read(n->device, header, offset, sizeof(posix_header));
        }
    }
    info->count = count;
    kfree(header);
    return OK;
}
error tarfs_find(vnode *parent, const char *name, vnode **found) {
    // VFS Node stuff
    tarfs_node *p = (tarfs_node *)parent;
    size_t offset = p->offset;

    // We read the header
    posix_header header;
    vfs_read(p->device, &header, offset, sizeof(header));

    // Offset 0 means root
    size_t parent_slash = p->offset == 0 ? 0 : strcnt(header.name, '/');
    while (memcmp(header.magic, "ustar", 5) == 0) {
        // Get information
        size_t s = otob(header.size);
        size_t child_slash = strcnt(header.name, '/');

        // If its regular it wont have an ending slash
        // So we add it
        if (header.typeflag == TAR_REG) {
            child_slash++;
        }

        // If we have one more slash than the root
        if (parent_slash + 1 == child_slash) {
            // We get the name from the path
            char *n = strdup(header.name);
            size_t end_of_path = strlen(n);
            if (n[end_of_path - 1] == '/') {
                n[end_of_path - 1] = '\0';
            }
            char *last_slash = strrchr(n, '/');
            if (last_slash == nullptr) {
                last_slash = n;
            } else {
                last_slash++;
            }
            if (strcmp(last_slash, name) == 0) {
                tarfs_node *f = kmalloc(sizeof(tarfs_node));
                f->device = p->device;
                f->offset = offset;
                f->is_root = false;

                strcpy(f->node.name, name);

                f->node.ops = tarfs_ops;
                kfree(n);

                *found = (vnode *)f;
                return OK;
            }
            kfree(n);
        }
        // Skip past the header and data
        offset += (((s + 511) / 512) + 1) * 512;

        // We read the next header
        vfs_read(p->device, &header, offset, sizeof(header));
    }
    return NO_ENTRY;
}
error tarfs_entry(vnode *parent, size_t index, vnode **found) {
    // VFS Node stuff
    tarfs_node *p = (tarfs_node *)parent;
    size_t offset = p->offset;

    // We read the header
    posix_header header;
    vfs_read(p->device, &header, offset, sizeof(header));

    // Index
    size_t i = 0;
    // Offset 0 means root
    size_t parent_slash = p->offset == 0 ? 0 : strcnt(header.name, '/');
    while (memcmp(header.magic, "ustar", 5) == 0) {
        // Get information
        size_t s = otob(header.size);
        size_t child_slash = strcnt(header.name, '/');

        // If its regular it wont have an ending slash
        // So we add it
        if (header.typeflag == TAR_REG) {
            child_slash++;
        }

        // If we have one more slash than the root
        if (parent_slash + 1 == child_slash) {
            if (i == index) {
                tarfs_node *f = kmalloc(sizeof(tarfs_node));
                f->device = p->device;
                f->offset = offset;
                f->is_root = false;

                // We get the name from the path
                char *name = strdup(header.name);
                size_t end_of_path = strlen(name);
                if (name[end_of_path - 1] == '/') {
                    name[end_of_path - 1] = '\0';
                }
                char *last_slash = strrchr(name, '/');
                if (last_slash == nullptr) {
                    last_slash = name;
                } else {
                    last_slash++;
                }
                strcpy(f->node.name, last_slash);

                f->node.ops = tarfs_ops;
                kfree(name);

                *found = (vnode *)f;
                return OK;
            } else {
                i++;
            }
        }
        // Skip past the header and data
        offset += (((s + 511) / 512) + 1) * 512;

        // We read the next header
        vfs_read(p->device, &header, offset, sizeof(header));
    }
    return NO_ENTRY;
}
error tarfs_create(vnode *parent, const char *name, vtype type, vflags flags) {
    return OK;
}
error tarfs_rename(vnode *node, const char *new_name) {
    return OK;
}
error tarfs_delete(vnode *node) {
    return OK;
}
