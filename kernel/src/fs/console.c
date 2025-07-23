#include "dev/keyboard.h"
#include <fs/console.h>
#include <fs/devfs.h>
#include <fs/vfs.h>
#include <lib/hashmap.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <mm/heap.h>

error console_open(vnode *node, vflags flags);
error console_read(vnode *node, void *buffer, size_t offset, size_t size);
error console_write(vnode *node, const void *buffer, size_t offset, size_t size);
error console_close(vnode *node);
error console_info(vnode *node, vinfo *info);

typedef struct {
    devfs_node node;
} console_node;

vnode_ops console_ops = {
    .open = console_open,
    .read = console_read,
    .write = console_write,
    .close = console_close,
    .info = console_info
};
error console_open(vnode *node, vflags flags) {
    (void)node;
    (void)flags;
    return OK;
}
error console_read(vnode *node, void *buffer, size_t offset, size_t size) {
    (void)node;
    (void)offset;
    char *s = buffer;
    for (size_t i = 0; i < size; i++) {
        s[i] = keyboard_read();
    }
    return OK;
}
error console_write(vnode *node, const void *buffer, size_t offset, size_t size) {
    (void)node;
    (void)offset;

    const char *s = buffer;
    for (size_t i = 0; i < size; i++) {
        write_char(s[i]);
    }
    return OK;
}
error console_close(vnode *node) {
    (void)node;
    return OK;
}
error console_info(vnode *node, vinfo *info) {
    (void)node;
    memset(info, 0, sizeof(vinfo));
    return OK;
}
void console_init() {
    console_node *n = heap_alloc(sizeof(console_node));
    n->node.node.ops = console_ops;
    strcpy(n->node.node.name, "console");

    devfs_add((vnode *)n);
    LOG("Console Device Initialized");
}
