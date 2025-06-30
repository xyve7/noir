// The goal of this tool is to extend the TAR format to support mountpoints
// This is an incredibly lazy fix
// But it makes things a lot easier for me

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char reserved[12];
} posix_header;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "noir-rd: insufficent number of arguments\n");
        return EXIT_FAILURE;
    }

    // Get the arguments
    int opt;
    // D means dir, R means regular, M means mount
    char *options = "DRM";

    // Flags
    int flag;
    char *path = nullptr;
    while ((opt = getopt(argc, argv, "p:f:")) != -1) {
        switch (opt) {
        case 'f':
            if (strchr(options, optarg[0]) == nullptr) {
                fprintf(stderr, "noir-rd: unknown flag: %s\n", optarg);
                return EXIT_FAILURE;
            }
            flag = optarg[0];
            break;
        case 'p':
            path = strdup(optarg);
            break;
        }
    }

    char *tarball = argv[optind];
    // Load the file into memory
    FILE *fp = fopen(tarball, "rb");

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    void *buffer = malloc(size);
    fread(buffer, 1, size, fp);

    fclose(fp);

    void *current = buffer;
    posix_header *header = current;
    while (memcmp(header->magic, "ustar", 5) == 0) {
        // Size of the file
        size_t s = strtol(header->size, nullptr, 8);

        // Compare the file path
        if (strcmp(header->name, path) == 0) {
            char typeflag;
            switch (flag) {
            case 'D':
                typeflag = '5';
                break;
            case 'R':
                typeflag = '0';
                break;
            case 'M':
                typeflag = '8'; // noir extension
                break;
            }

            // We modify the typeflag
            header->typeflag = typeflag;
        }

        current += (((s + 511) / 512) + 1) * 512;
        header = current;
    }

    // We write the buffer back
    fp = fopen(tarball, "wb");
    fwrite(buffer, 1, size, fp);
    fclose(fp);

    free(path);
    return EXIT_SUCCESS;
}
