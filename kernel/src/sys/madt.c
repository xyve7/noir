#include <kernel.h>
#include <stdint.h>
#include <sys/acpi.h>
#include <sys/madt.h>

vec lapics;
vec ioapics;
vec ioapic_overrides;
vec ioapic_nmis;
vec lapic_nmis;
vec lapic_overrides;
vec x2apics;

void madt_init() {
    madt *entry = (madt *)acpi_get_entry("APIC");
    if (entry == nullptr) {
        PANIC("madt not found");
    }

    // Initialize every vector
    lapics = vec_new();
    ioapics = vec_new();
    ioapic_overrides = vec_new();
    ioapic_nmis = vec_new();
    lapic_nmis = vec_new();
    lapic_overrides = vec_new();
    x2apics = vec_new();

    // The entries exist after the entry, so we add to it
    // And then after, we get the address, and advance it based on the size

    // The size of every entry
    uint32_t length = entry->header.length - sizeof(*entry);
    madt_entry_header *madt_entry = (madt_entry_header *)(entry + 1);
    while (length > 0) {
        uint8_t size = madt_entry->length;
        uint8_t type = madt_entry->type;
        switch (type) {
        case MADT_TYPE_LOCAL_APIC: {
            madt_lapic *lapic = (madt_lapic *)madt_entry;
            vec_push(&lapics, lapic);
            break;
        }
        case MADT_TYPE_IO_APIC: {
            madt_ioapic *ioapic = (madt_ioapic *)madt_entry;
            vec_push(&ioapics, ioapic);
            break;
        }
        case MADT_TYPE_INT_SRC_OVERRIDE: {
            madt_ioapic_int_override *ioapic_override = (madt_ioapic_int_override *)madt_entry;
            vec_push(&ioapic_overrides, ioapic_override);
            break;
        }
        case MADT_TYPE_IO_APIC_NMI: {
            madt_ioapic_nmi *ioapic_nmi = (madt_ioapic_nmi *)madt_entry;
            vec_push(&ioapic_nmis, ioapic_nmi);
            break;
        }
        case MADT_TYPE_LOCAL_APIC_NMI: {
            madt_lapic_nmi *lapic_nmi = (madt_lapic_nmi *)madt_entry;
            vec_push(&lapic_nmis, lapic_nmi);
            break;
        }
        case MADT_TYPE_LOCAL_APIC_ADDR_OVR: {
            madt_lapic_override *lapic_override = (madt_lapic_override *)madt_entry;
            vec_push(&lapic_overrides, lapic_override);
            break;
        }
        case MADT_TYPE_LOCAL_X2APIC: {
            madt_x2apic *x2apic = (madt_x2apic *)madt_entry;
            vec_push(&x2apics, x2apic);
            break;
        }
        default:
            PANIC("Unknown MADT entry: %hhu\n", type);
            break;
        }
        // Remove from the length
        length -= size;
        // Advance the pointer and reassign
        void *temp = madt_entry;
        madt_entry = temp + size;
    }

    LOG("MADT Parsed");
}
