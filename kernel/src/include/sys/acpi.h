#pragma once
#include <stdint.h>

typedef struct [[gnu::packed]] {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} rsdp;

typedef struct [[gnu::packed]] {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t ext_checksum;
    uint8_t reserved[3];
} xsdp;

typedef enum {
    RS,
    XS
} acpi_kind;

typedef struct [[gnu::packed]] {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_tableid[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} sdt_header;

typedef struct [[gnu::packed]] {
    sdt_header header;
    uint32_t sdts[];
} rsdt;

typedef struct [[gnu::packed]] {
    sdt_header header;
    uint64_t sdts[];
} xsdt;

void acpi_init();
sdt_header *acpi_get_entry(char *signature);
