#pragma once

#include <fs/vfs.h>
#include <lib/hashmap.h>

typedef struct {
    vnode node;
    hashmap children;
} devfs_node;

void devfs_init();
void devfs_add(vnode *v);
