#include <kernel.h>
#include <lib/printf.h>
#include <lib/string.h>
#include <limine.h>
#include <mm/vmm.h>
#include <stddef.h>
#include <sys/acpi.h>

__attribute__((used, section(".limine_requests"))) static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

rsdp *rsdp_ptr;
xsdp *xsdp_ptr;

bool is_extended = false;

rsdt *rsdt_ptr;
xsdt *xsdt_ptr;

void acpi_init() {
    // By default, assume rsdp
    rsdp_ptr = (void *)VIRT(rsdp_request.response->address);
    LOG("%p\n", rsdp_ptr);
    if (rsdp_ptr->revision == 2) {
        // If the revision is 2, assume xsdp
        is_extended = true;
        xsdp_ptr = (void *)VIRT(rsdp_request.response->address);
    }

    // Get the sdt, either xsdt or rsdt
    if (is_extended) {
        xsdt_ptr = (xsdt *)VIRT(xsdp_ptr->xsdt_address);
    } else {
        rsdt_ptr = (rsdt *)VIRT(rsdp_ptr->rsdt_address);
    }
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
