#include <kernel.h>
#include <lib/string.h>
#include <lib/vec.h>
#include <mm/heap.h>
#include <sys/vfs.h>

vfs_node *root = nullptr;

void vfs_init(char *name) {
    root = kmalloc(sizeof(*root));

    root->name = strdup(name);
    root->kind = VFS_DIR;
    root->open = nullptr;
    root->write = nullptr;
    root->read = nullptr;
    root->close = nullptr;

    root->children = hashmap_new();
}
status vfs_get(char *path, vfs_node **node) {
    if (root == nullptr) {
        PANIC("no vfs initialized\n");
    }

    // Check if it starts with /
    if (*path != '/') {
        PANIC("invalid path: %s\n");
    }

    // Check if we are the root
    if (strcmp(path, "/") == 0) {
        return EXISTS;
    }

    status ret = OK;

    // Duplicated since strtok modifies it
    // Split the path
    vec path_parts = vec_new();
    char *p = strdup(path);
    char *token = strtok(p, "/");
    while (token) {
        vec_push(&path_parts, token);
        token = strtok(nullptr, "/");
    }

    // Iterate through everything
    vfs_node *current = root;
    for (size_t i = 0; i < path_parts.len; i++) {
        // Current part
        char *part = vec_at(&path_parts, i);
        vfs_node *node = hashmap_get(&current->children, part)->value;
        if (node) {
            current = node;
        } else {
            // No entry found
            ret = NO_ENTRY;
            goto cleanup;
        }
    }

    *node = current;
cleanup:
    vec_free(&path_parts);
    kfree(p);
    return ret;
}
status vfs_create(char *path, vfs_node_kind kind, void *data, vfs_open_op open, vfs_write_op write, vfs_read_op read, vfs_close_op close) {
    if (root == nullptr) {
        PANIC("no vfs initialized\n");
    }

    // Check if it starts with /
    if (*path != '/') {
        PANIC("invalid path: %s\n");
    }

    // Check if we are the root
    if (strcmp(path, "/") == 0) {
        return EXISTS;
    }

    status ret = OK;

    // Duplicated since strtok modifies it
    // Split the path
    vec path_parts = vec_new();
    char *p = strdup(path);
    char *token = strtok(p, "/");
    while (token) {
        vec_push(&path_parts, token);
        token = strtok(nullptr, "/");
    }

    // Iterate through everything but the last
    vfs_node *current = root;
    for (size_t i = 0; i < path_parts.len - 1; i++) {
        // Current part
        char *part = vec_at(&path_parts, i);
        vfs_node *node = hashmap_get(&current->children, part)->value;
        if (node) {
            current = node;
        } else {
            // No entry found
            ret = NO_ENTRY;
            goto cleanup;
        }
    }

    // The current node is the parent, make it
    char *name = vec_at(&path_parts, path_parts.len - 1);
    vfs_node *new_node = kmalloc(sizeof(vfs_node));
    new_node->parent = current;
    new_node->name = strdup(name);
    new_node->kind = kind;
    new_node->data = data;
    new_node->open = open;
    new_node->write = write;
    new_node->read = read;
    new_node->close = close;
    new_node->children = hashmap_new();

    // Add it
    hashmap_add(&current->children, strdup(name), new_node, sizeof(*new_node));
cleanup:
    vec_free(&path_parts);
    kfree(p);
    return ret;
}

status vfs_open(vfs_node *self) {
    return self->open(self->data);
}
status vfs_write(vfs_node *self, const void *buffer, size_t bytes, size_t *written_bytes) {
    return self->write(self->data, buffer, bytes, written_bytes);
}
status vfs_read(vfs_node *self, void *buffer, size_t bytes, size_t *read_bytes) {
    return self->read(self->data, buffer, bytes, read_bytes);
}
status vfs_close(vfs_node *self) {
    return self->close(self->data);
}
