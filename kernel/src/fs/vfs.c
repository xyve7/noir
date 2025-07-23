#include <fs/vfs.h>
#include <lib/hashmap.h>
#include <lib/string.h>
#include <lib/string_buffer.h>
#include <lib/vector.h>
#include <mm/heap.h>
#include <stdarg.h>

// Normalize a path
// Remove duplicate slashes, resolve . and ..
// "/home/user/dir/../dir2/file.txt" -> /home/user/dir2/file.txt"
// NOTE: The caller is responsible for freeing the string
char *path_normalize(const char *path) {
    // This is so strtok doesn't cause UB
    char *duped = strdup(path);

    vector parts = vector_new();
    char *token = strtok(duped, "/");
    while (token) {
        if (strncmp(token, "..", 2) == 0) {
            // This means we aren't at the root
            if (parts.len > 0) {
                // We pop the previous directory off the path.
                // This is essentially moving back a directory.
                vector_pop(&parts);
            }
            // We skip going up a directory if its at the root
        } else if (strncmp(token, ".", 1) != 0) {
            // We push the token that isn't "."
            vector_push(&parts, token);
        }

        // "." is implicitly handled
        // Since "." won't wont be pushed, its going to be ignored
        token = strtok(nullptr, "/");
    }

    string_buffer norm = string_buffer_new();
    // This means we have just a root
    if (parts.len < 1) {
        string_buffer_push_ch(&norm, '/');
    } else {
        for (size_t i = 0; i < parts.len; i++) {
            char *s = vector_at(&parts, i);
            string_buffer_push_ch(&norm, '/');
            string_buffer_push_str(&norm, s);
        }
    }

    heap_free(duped);
    vector_free(&parts);

    return norm.data;
}
// Join all the parts of a path
// Example:
// call: path_from_parts("home", "user", nullptr)
// return: "/home/user"
// NOTE: The caller is responsible for freeing the string
char *path_from_parts(...) {
    va_list args;
    va_start(args);

    // Create a new buffer
    string_buffer joined = string_buffer_new();

    // We keep adding the args
    // The args are terminated with nullptr
    const char *arg = va_arg(args, const char *);
    while (arg) {
        string_buffer_push_ch(&joined, '/');
        string_buffer_push_str(&joined, arg);
        // Next argument
        arg = va_arg(args, const char *);
    }

    va_end(args);

    return joined.data;
}

// Get the parent path
// path: /home/user/file.txt
// return: /home/user
// NOTE: The caller is responsible for freeing the string
char *path_parent(const char *path) {
    // We duplicate the path so we can modify it
    char *duped = strdup(path);
    // We get the last slash
    //			  /home/user/file.txt
    // lash_slash ----------^
    char *last_slash = strrchr(duped, '/');

    // If we are at the root, or the parent is root
    // This is because:
    //		        /home
    // duped -------^
    // last_slash --^
    if (last_slash == duped) {
        // We move past the slash and nul
        *(last_slash + 1) = '\0';
    } else {
        // We nul the slash to terminate it
        *last_slash = '\0';
    }
    // We return the duped string
    return duped;
}
char *path_basename(const char *path) {
    // We get the last slash
    //			  /home/user/file.txt
    // lash_slash ----------^
    char *last_slash = strrchr(path, '/');
    // We go past the slash, getting the basename
    last_slash++;

    // We duplicate it until the nul terminator
    return strdup(last_slash);
}

// Initially, I had a mount node like this:
// typedef struct {
//     char name[VFS_NAME_MAX];
//     vmount *value;
//     hashmap children;
// } mount_node;
//
// This was ridiculously complicated.
// The whole tree structure was unnecessary to traverse.
// So I opted to have just a hashmap with a mount node.
// The path being the key of course
//
// "/" -> mount
// "/system" -> mount

// Mount table
hashmap mount_table;
// Filesystem table
hashmap filesystems;

// Add a mountpoint to the table
error vfs_add_mountpoint(const char *where, vmount *mountpoint) {
    // Fetch the mount point
    hashmap_object *object = hashmap_get(&mount_table, where);
    // We have an object
    if (object) {
        return EXISTS;
    }

    // Not already mounted
    hashmap_set(&mount_table, where, mountpoint, sizeof(*mountpoint));
    return OK;
}
// Fetch a mount point from the table
error vfs_get_mountpoint(const char *where, vmount **mountpoint) {
    // Fetch the mount point
    hashmap_object *object = hashmap_get(&mount_table, where);
    // We have an object
    if (object == nullptr) {
        return NO_ENTRY;
    }
    // Return the mount point
    *mountpoint = object->value;
    return OK;
}
// Lookup the path within the entire VFS
// NOTE: Accepts a normalized path
error vfs_lookup(const char *path, vnode **found) {
    // The way this thing works is that we check for a mountpoint every time we traverse.
    // We get a node, pass in the child's name.
    //
    // path: /home
    // home_node: node("/").find("home")
    //
    // The find is implemented by the filesystem itself.

    error err = OK;
    char *duped = strdup(path);

    // We save memory here by doing this weird trick
    char *current_path = strdup(duped);
    // We skip the leading /
    char *path_end = current_path + strspn(current_path, "/");
    // The path_end is where the path currently ends

    // Since strtok won't get the actual slash
    // We manually get the root
    vmount *root_mount;
    err = vfs_get_mountpoint("/", &root_mount);
    // Filesystem is not mounted
    if (err != OK) {
        goto cleanup;
    }

    // We tokenize the string
    char *token = strtok(duped, "/");

    vnode *current = root_mount->root;
    while (token) {
        // We search within the first node
        // find: "home" within "/"
        vnode *found;
        err = current->ops.find(current, token, &found);
        // Error, didnt find it
        if (err != OK) {
            goto cleanup;
        }

        // We check if the node is a mount point
        vinfo info;
        err = found->ops.info(found, &info);
        // Error, didnt find it
        if (err != OK) {
            goto cleanup;
        }

        bool reached_end = false;
        // We go to the next slash
        path_end = strchr(path_end, '/');
        // We are going to get a nullptr if we reach the end
        // If we do, we just set the boolean so we don't undo the nul termination
        reached_end = path_end == nullptr;
        if (!reached_end) {
            *path_end = '\0';
        }

        // The entry is a mount point
        if (info.type == VFS_MOUNT) {
            vmount *m;
            err = vfs_get_mountpoint(current_path, &m);
            // Nothing is mounted at the mountpoint
            if (err != OK) {
                goto cleanup;
            }
            // We got the root of the filesystem
            // So instead of pointing to the mountpoint in the current filesystem.
            // We move to the filesystem pointed by the mount point.
            // Essentially, we are handing off to the mounted filesystem.
            found = m->root;
        }

        // Next node
        current = found;

        // Next token
        token = strtok(nullptr, "/");

        // Wait, before we go!
        // We undo the nul termination
        if (!reached_end) {
            *path_end = '/';
        }
    }

    *found = current;

cleanup:
    heap_free(duped);
    heap_free(current_path);
    return err;
}
// Open a VFS
error vfs_open(const char *path, vflags flags, vnode **result) {
    // We normalize the path
    char *normalized = path_normalize(path);

    // Lookup
    vnode *found;
    error err = vfs_lookup(normalized, &found);

    // Handle error
    if (err != OK) {
        goto cleanup;
    }

    // Open operation is not supported
    if (found->ops.open == nullptr) {
        err = BAD_OP;
        goto cleanup;
    }
    err = found->ops.open(found, flags);

    // Return the node we found
    *result = found;
cleanup:
    heap_free(normalized);
    return err;
}
error vfs_read(vnode *node, void *buffer, size_t offset, size_t size) {
    if (node->ops.read) {
        return node->ops.read(node, buffer, offset, size);
    }
    return BAD_OP;
}
error vfs_write(vnode *node, const void *buffer, size_t offset, size_t size) {
    if (node->ops.write) {
        return node->ops.write(node, buffer, offset, size);
    }
    return BAD_OP;
}
error vfs_close(vnode *node) {
    if (node->ops.close) {
        return node->ops.close(node);
    }
    return BAD_OP;
}

// These calls are to be determined
error vfs_info(const char *path, vinfo *info) {
    // We normalize the path
    char *normalized = path_normalize(path);

    // Lookup
    vnode *found;
    error err = vfs_lookup(normalized, &found);

    // Handle error
    if (err != OK) {
        goto cleanup;
    }

    // Info operation is not supported
    if (found->ops.info == nullptr) {
        err = BAD_OP;
        goto cleanup;
    }
    vinfo i;
    err = found->ops.info(found, &i);

    // Return the node we found
    *info = i;
cleanup:
    heap_free(normalized);
    return err;
}
error vfs_entry(const char *path, size_t index, vnode **found) {
    // We normalize the path
    char *normalized = path_normalize(path);

    // Lookup
    vnode *f1;
    error err = vfs_lookup(normalized, &f1);

    // Handle error
    if (err != OK) {
        goto cleanup;
    }

    // Info operation is not supported
    if (f1->ops.entry == nullptr) {
        err = BAD_OP;
        goto cleanup;
    }
    vnode *f2;
    err = f1->ops.entry(f1, index, &f2);

    // Return the node we found
    *found = f2;
cleanup:
    heap_free(normalized);
    return err;
}

error vfs_map(vnode *node, size_t offset, size_t size, void **where) {
    if (node->ops.map) {
        return node->ops.map(node, offset, size, where);
    }
    return BAD_OP;
}
error vfs_create(const char *path, vtype type, vflags flags) {
    // We normalize the path
    char *normalized = path_normalize(path);
    char *parent = path_parent(normalized);
    char *basename = path_basename(normalized);

    // Lookup
    vnode *f;
    error err = vfs_lookup(parent, &f);

    if (err != OK) {
        goto cleanup;
    }

    err = f->ops.create(f, basename, type, flags);

cleanup:
    heap_free(normalized);
    heap_free(parent);
    heap_free(basename);
    return err;
}
error vfs_rename(const char *old_name, const char *new_name);
error vfs_delete(vnode *node) {
    if (node->ops.delete) {
        return node->ops.delete(node);
    }
    return BAD_OP;
}

error vfs_register(vfs *fs) {
    // Get name
    char *s = fs->name;
    // Get the actual fs
    hashmap_object *object = hashmap_get(&filesystems, s);
    // Filesystem exists
    if (object != nullptr) {
        return EXISTS;
    }
    // Doesn't exist, set filesystem
    hashmap_set(&filesystems, s, fs, sizeof(*fs));
    return OK;
}

error vfs_mount(const char *device, const char *where, const char *fs) {
    hashmap_object *object = hashmap_get(&filesystems, fs);
    if (object == nullptr) {
        return BAD_FS;
    }

    // Get filesystem
    vfs *filesystem = object->value;
    if (filesystem->ops.mount == nullptr) {
        return BAD_OP;
    }

    // Find the device
    vnode *device_node = nullptr;
    error err = vfs_lookup(device, &device_node);
    if (err != OK) {
        return err;
    }

    // Get the mountpoint
    vmount *mountpoint;
    filesystem->ops.mount(device_node, &mountpoint);

    // Add the mountpoint
    vfs_add_mountpoint(where, mountpoint);

    return OK;
}
error vfs_mount_raw(vnode *device, const char *where, const char *fs) {
    hashmap_object *object = hashmap_get(&filesystems, fs);
    if (object == nullptr) {
        return BAD_FS;
    }

    // Get filesystem
    vfs *filesystem = object->value;
    if (filesystem->ops.mount == nullptr) {
        return BAD_OP;
    }

    // Get the mountpoint
    vmount *mountpoint;
    filesystem->ops.mount(device, &mountpoint);

    // Add the mountpoint
    vfs_add_mountpoint(where, mountpoint);

    return OK;
}
error vfs_init() {
    mount_table = hashmap_new();
    filesystems = hashmap_new();

    LOG("VFS Initialized");

    return OK;
}
