#pragma once

#include <lib/hashmap.h>
#include <stddef.h>

typedef enum {
    OK,
    NO_ENTRY,
    EXISTS
} status;

typedef status (*vfs_open_op)(void *);
typedef status (*vfs_write_op)(void *, const void *, size_t, size_t *);
typedef status (*vfs_read_op)(void *, void *, size_t, size_t *);
typedef status (*vfs_close_op)(void *);

typedef enum {
    VFS_FILE,
    VFS_DIR,
    VFS_DEV
} vfs_node_kind;

typedef struct vfs_node {
    struct vfs_node *parent;

    char *name;
    vfs_node_kind kind;

    void *data;

    vfs_open_op open;
    vfs_write_op write;
    vfs_read_op read;
    vfs_close_op close;

    hashmap children;
} vfs_node;

extern vfs_node *root;

void vfs_init(char *name);
status vfs_get(char *path, vfs_node **node);
status vfs_create(char *path, vfs_node_kind kind, void *data, vfs_open_op open, vfs_write_op write, vfs_read_op read, vfs_close_op close);

status vfs_open(vfs_node *self);
status vfs_write(vfs_node *self, const void *buffer, size_t bytes, size_t *written_bytes);
status vfs_read(vfs_node *self, void *buffer, size_t bytes, size_t *read_bytes);
status vfs_close(vfs_node *self);
