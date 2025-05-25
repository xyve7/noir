#pragma once

#include <stdint.h>
#include <sys/acpi.h>

typedef struct [[gnu::packed]] {
    uint8_t type;
    uint8_t length;
} madt_entry_header;

typedef struct [[gnu::packed]] {
    madt_entry_header header;
    uint8_t proc_id;
    uint8_t id;
    uint32_t flags;
} madt_lapic;

typedef struct [[gnu::packed]] {
    madt_entry_header header;
    uint8_t id;
    uint8_t reserved;
    uint32_t address;
    uint32_t gsi_base;
} madt_ioapic;

typedef struct [[gnu::packed]] {
    madt_entry_header header;
    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t gsi;
    uint16_t flags;
} madt_ioapic_int_override;

typedef struct [[gnu::packed]] {
    madt_entry_header header;
    uint8_t src;
    uint8_t reserved;
    uint16_t flags;
    uint32_t gsi;
} madt_ioapic_nmi;

typedef struct [[gnu::packed]] {
    madt_entry_header header;
    uint8_t proc_id;
    uint16_t flags;
    uint8_t lint;
} madt_lapic_nmi;

typedef struct [[gnu::packed]] {
    madt_entry_header header;
    uint16_t reserved;
    uint64_t address;
} madt_lapic_override;

typedef struct [[gnu::packed]] {
    madt_entry_header header;
    uint16_t reserved;
    uint32_t proc_id;
    uint32_t flags;
    uint32_t acpi_id;
} madt_x2apic;

typedef struct [[gnu::packed]] {
    sdt_header header;
    uint32_t lapic_address;
    uint32_t flags;
} madt;
