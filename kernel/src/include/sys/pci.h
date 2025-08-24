#pragma once

#include <stdint.h>
typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t class;
    uint8_t subclass;
    uint8_t progif;
    uint8_t revision;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t header;
} pci_device;

void pci_init();
