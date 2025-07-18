#include "mm/pmm.h"
#include "mm/vmm.h"
#include <fs/devfs.h>
#include <fs/vfs.h>
#include <kernel.h>
#include <lib/hashmap.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <mm/heap.h>
#include <stdint.h>
error framebuffer_open(vnode *node, vflags flags);
error framebuffer_close(vnode *node);
error framebuffer_info(vnode *node, vinfo *info);
error framebuffer_map(vnode *parent, size_t offset, size_t size, void **where);

typedef struct {
    devfs_node node;
} framebuffer_node;

vnode_ops framebuffer_ops = {
    .open = framebuffer_open,
    .close = framebuffer_close,
    .info = framebuffer_info,
    .map = framebuffer_map,
};
error framebuffer_open(vnode *node, vflags flags) {
    (void)node;
    (void)flags;
    return OK;
}
error framebuffer_close(vnode *node) {
    (void)node;
    return OK;
}
error framebuffer_info(vnode *node, vinfo *info) {
    (void)node;
    memset(info, 0, sizeof(vinfo));
    info->size = framebuffer->height * framebuffer->pitch;
    return OK;
}

error framebuffer_map(vnode *node, size_t offset, size_t size, void **where) {
    return OK;
}
void framebuffer_init() {
    framebuffer_node *n = kmalloc(sizeof(framebuffer_node));
    n->node.node.ops = framebuffer_ops;
    strcpy(n->node.node.name, "framebuffer");

    devfs_add((vnode *)n);

    LOG("Framebuffer Device Initialized");
}
