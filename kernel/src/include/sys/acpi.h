#pragma once
#include <stdint.h>

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

// Initialize ACPI
void acpi_init();
// Get sdt_header of the specified signature
// Ex: "APIC", "HPET"
sdt_header *acpi_get_entry(char *signature);
