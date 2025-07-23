#include <fs/devfs.h>
#include <fs/vfs.h>
#include <lib/hashmap.h>
#include <lib/string.h>
#include <mm/heap.h>

error devfs_mount(vnode *device, vmount **mount);
error devfs_find(vnode *parent, const char *name, vnode **found);
error devfs_entry(vnode *parent, size_t index, vnode **found);

vfs_ops devfs_vfs_ops = {
    .mount = devfs_mount
};
vnode_ops devfs_ops = {
    .find = devfs_find,
    .entry = devfs_entry,
};

vfs *devfs_vfs = nullptr;
devfs_node *root = nullptr;

void devfs_init() {
    // Create VFS
    devfs_vfs = heap_alloc(sizeof(vfs));
    strcpy(devfs_vfs->name, "devfs");
    devfs_vfs->ops = devfs_vfs_ops;

    root = heap_alloc(sizeof(devfs_node));
    memset(root, 0, sizeof(*root));
    root->node.ops = devfs_ops;
    root->children = hashmap_new();

    vfs_register(devfs_vfs);
    vfs_mount_raw(nullptr, "/device", "devfs");

    LOG("Device Filesystem Initialized");
}
void devfs_add(vnode *v) {
    hashmap_set(&root->children, v->name, v, sizeof(*v));
}
error devfs_mount(vnode *device, vmount **mount) {
    // It is the devfs, device doesn't matter
    (void)device;

    // Create the mount point
    vmount *m = heap_alloc(sizeof(vmount));
    m->root = (vnode *)root;
    m->fs = devfs_vfs;
    
    *mount = m;

    return OK;
}

error devfs_find(vnode *parent, const char *name, vnode **found) {
    devfs_node *n = (devfs_node *)parent;
    hashmap_object *element = hashmap_get(&n->children, name);
    if (element == nullptr) {
        *found = nullptr;
        return NO_ENTRY;
    }
    *found = element->value;
    return OK;
}
error devfs_entry(vnode *parent, size_t index, vnode **found) {
    devfs_node *n = (devfs_node *)parent;
    hashmap_object *element = hashmap_at(&n->children, index);
    if (element == nullptr) {
        *found = nullptr;
        return NO_ENTRY;
    }
    *found = element->value;
    return OK;
}
