#ifndef NOIR_H
#define NOIR_H

#include <stdint.h>

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
} Error;

typedef struct {
    uint8_t patch, minor, major;
    char name[8];
    char revision[8];
    char timestamp[128];
    char build[32];
} Version;

typedef uint64_t FD;
typedef uint64_t PID;

#endif
