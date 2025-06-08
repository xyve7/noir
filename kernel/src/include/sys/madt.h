#pragma once

#include <lib/vec.h>
#include <stdint.h>
#include <sys/acpi.h>

// Processor Local APIC
#define MADT_TYPE_LOCAL_APIC 0
// I/O APIC
#define MADT_TYPE_IO_APIC 1
// Interrupt Source Override
#define MADT_TYPE_INT_SRC_OVERRIDE 2
// I/O APIC Non-maskable Interrupt
#define MADT_TYPE_IO_APIC_NMI 3
// Local APIC Non-maskable Interrupt
#define MADT_TYPE_LOCAL_APIC_NMI 4
// Local APIC Address Override
#define MADT_TYPE_LOCAL_APIC_ADDR_OVR 5
// Processor Local x2APIC
#define MADT_TYPE_LOCAL_X2APIC 9

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

extern vec lapics;
extern vec ioapics;
extern vec ioapic_overrides;
extern vec ioapic_nmis;
extern vec lapic_nmis;
extern vec lapic_overrides;
extern vec x2apics;

void madt_init();
