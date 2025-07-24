#pragma once

typedef unsigned char uint8_t;
typedef unsigned long uint64_t;
typedef unsigned long size_t;

// Errors returned to the user
typedef enum : uint64_t {
    // No action needed, successful
    OK,
    /* I/O */
    // Entry doesn't exist
    NO_ENTRY,
    // Entry already exists
    EXISTS,
    // File descriptor doesn't exist
    BAD_FD,
    // Entry is directory
    IS_DIR,
    // Entry is file
    IS_FILE,
    // Entry is device
    IS_DEV,
    // Read only
    READ_ONLY,
    // Write only
    WRITE_ONLY,
    // Unsupported operations
    BAD_OP,
    // Unsupported filesystem
    BAD_FS
} error;

typedef struct {
    uint8_t patch, minor, major;
    char name[8];
    char revision[8];
    char timestamp[128];
    char build[32];
} noir_version;
