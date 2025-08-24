#include <cpu/cpu.h>
#include <dev/serial.h>
#include <sys/pci.h>

#define CONFIG_ADDRESS 0xcf8
#define CONFIG_DATA 0xcfc

// The base address we build on, the enable bit is set since...
// "Bit 31 is an enable flag for determining when
// accesses to CONFIG_DATA are to be translated to configuration transactions on the PCI
// bus."
// - PCI Local Bus Specification Revision 2.2
//   Section 3.2.2.3.2. (Software Generation of Configuration Transactions)

#define CONFIG_ADDRESS_BASE 0x80000000

// Read a 32-bit value from the PCI config
uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    // The offset's granularity is 4 bytes (DWORD)
    // We shift it since the first 2 bits have to be zero
    uint32_t address = CONFIG_ADDRESS_BASE | (bus << 16) | (device << 11) | (function << 8) | (offset << 2);
    outl(CONFIG_ADDRESS, address);
    return inl(CONFIG_DATA);
}

void pci_init() {
    // We brute force for now
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t reg0 = pci_config_read(bus, device, function, 0);
                uint32_t reg2 = pci_config_read(bus, device, function, 2);
                uint32_t reg3 = pci_config_read(bus, device, function, 3);

                // Register 0
                uint16_t vendor_id = reg0 & 0xFFFF;
                uint16_t device_id = (reg0 >> 16) & 0xFFFF;

                // Register 2
                uint8_t class = (reg2 >> 24) & 0xFF;
                uint8_t subclass = (reg2 >> 16) & 0xFF;
                uint8_t prog_if = (reg2 >> 8) & 0xFF;
                uint8_t revision = reg2 & 0xFF;

                // Register 3
                uint8_t header = (reg3 >> 16) & 0xFF;

                // If its valid
                if (vendor_id != 0xFFFF) {
                    serial_printf("PCI {\n");
                    serial_printf("\tbus: %hhx\n", bus);
                    serial_printf("\tdevice: %hhx\n", device);
                    serial_printf("\tfunction: %hhx\n", function);
                    serial_printf("\tclass: %hhx\n", class);
                    serial_printf("\tsubclass: %hhx\n", subclass);
                    serial_printf("\tprog_if: %hhx\n", prog_if);
                    serial_printf("\trevision: %hhx\n", revision);
                    serial_printf("\tvendor_id: %hx\n", vendor_id);
                    serial_printf("\tdevice_id: %hx\n", device_id);
                    serial_printf("\theader: %hhx\n", header);
                    serial_printf("}\n");
                }
            }
        }
    }
}
