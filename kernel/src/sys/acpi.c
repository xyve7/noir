#include <kernel.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <limine.h>
#include <mm/vmm.h>
#include <stddef.h>
#include <sys/acpi.h>

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

typedef struct [[gnu::packed]] {
    sdt_header header;
    uint32_t sdts[];
} rsdt;

typedef struct [[gnu::packed]] {
    sdt_header header;
    uint64_t sdts[];
} xsdt;

__attribute__((used, section(".limine_requests"))) static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

bool is_extended = false;

rsdt *rsdt_ptr;
xsdt *xsdt_ptr;

void acpi_init() {
    // By default, assume rsdp
    rsdp *rsdp_ptr = (void *)VIRT(rsdp_request.response->address);
    xsdp *xsdp_ptr = nullptr;

    // If the revision is 2, its an xsdp
    if (rsdp_ptr->revision == 2) {
        is_extended = true;
        xsdp_ptr = (void *)VIRT(rsdp_request.response->address);
        xsdt_ptr = (xsdt *)VIRT(xsdp_ptr->xsdt_address);
    } else {
        rsdt_ptr = (rsdt *)VIRT(rsdp_ptr->rsdt_address);
    }

    LOG("ACPI Initialized");
}

sdt_header *acpi_get_entry(char *signature) {
    size_t entries;
    if (is_extended) {
        entries = (xsdt_ptr->header.length - sizeof(xsdt_ptr->header)) / 8;
    } else {
        entries = (rsdt_ptr->header.length - sizeof(rsdt_ptr->header)) / 4;
    }

    for (size_t i = 0; i < entries; i++) {
        sdt_header *header;
        if (is_extended) {
            header = (sdt_header *)VIRT(xsdt_ptr->sdts[i]);
        } else {
            header = (sdt_header *)VIRT(rsdt_ptr->sdts[i]);
        }
        if (memcmp(header->signature, signature, 4) == 0) {
            return header;
        }
    }
    return nullptr;
}
