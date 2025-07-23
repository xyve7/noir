#include <fs/ramdisk.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <lib/string.h>
#include <mm/heap.h>

// We define a node that just reads from memory
// So we can read it
typedef struct {
    vnode node;
    void *memory;
    size_t size;
} mem_node;

error mem_read(vnode *node, void *buffer, size_t offset, size_t size);

vnode_ops mem_ops = {
    .read = mem_read
};

error mem_read(vnode *node, void *buffer, size_t offset, size_t size) {
    mem_node *n = (mem_node *)node;
    memcpy(buffer, n->memory + offset, size);
    return OK;
}

void ramdisk_init(const char *fs) {
    void *tar = module_request.response->modules[0]->address;
    size_t size = module_request.response->modules[0]->size;

    mem_node *tar_buf_root = heap_alloc(sizeof(mem_node));
    tar_buf_root->node.ops = mem_ops;
    tar_buf_root->memory = tar;
    tar_buf_root->size = size;

    vfs_mount_raw((vnode *)tar_buf_root, "/", fs);
}
