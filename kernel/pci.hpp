//
// Created by Piotr on 08.05.2024.
//

#ifndef KITTY_OS_CPP_PCI_HPP
#define KITTY_OS_CPP_PCI_HPP

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>
#include <kernel/hal/x64/io.hpp>
#include <kstd/kstdio.hpp>
#include <type_traits>
#include <kdu/driver_entry.hpp>
#include <kdu/driver_ctrl.hpp>

struct pci_header_common
{
    uint16_t vendor_id, device_id, command, status;
    uint8_t revision_id, prog_if, subclass, class_code, cache_line_size, latency_timer, header_type, bist;
} __attribute__((packed));

struct pci_device_header
{
    pci_header_common common;
    uint32_t bar0, bar1, bar2, bar3, bar4, bar5, cardbus_cis_pointer;
    uint16_t subsystem_vendor_id, subsystem_id;
    uint32_t expansion_rom_base_address;
    uint8_t capabilities_pointer;
    uint8_t rsvd0;
    uint16_t rsvd1;
    uint32_t rsvd2;
    uint8_t interrupt_line, interrupt_pin, min_grant, max_latency;
} __attribute__((packed));

struct pci_pci_to_pci_header
{
    pci_header_common common;
    uint32_t bar0, bar1;
    uint8_t primary_bus_number, secondary_bus_number, subordinate_bus_number, secondary_latency_timer;
    uint8_t io_base, io_limit;
    uint16_t secondary_status;
    uint16_t memory_base, memory_limit;
    uint16_t prefetchable_memory_base, prefetchable_memory_limit;
    uint32_t prefetchable_base_upper_32_bits;
    uint32_t prefetchable_base_lower_32_bits;
    uint16_t io_base_upper_16_bits;
    uint16_t io_limit_upper_16_bits;
    uint8_t capability_pointer;
    uint8_t rsvd0;
    uint16_t rsvd1;
    uint32_t expansion_rom_base_address;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
} __attribute__((packed));

struct pci_pci_to_cardbus_header
{
    pci_header_common common;
    uint32_t cardbus_socket_base_address;
    uint8_t offset_of_capabilities_list;
    uint8_t reserved;
    uint16_t secondary_status;
    uint8_t pci_bus_number, cardbus_bus_number, subordinate_bus_number, cardbus_latency_timer;
    uint32_t memory_base_address0;
    uint32_t memory_limit0;
    uint32_t memory_base_address1;
    uint32_t memory_limit1;
    uint32_t io_base_address0;
    uint32_t io_limit0;
    uint32_t io_base_address1;
    uint32_t io_limit1;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
    uint16_t subsystem_device_id;
    uint16_t subsystem_vendor_id;
    uint32_t pc_card_legacy_mode_base_address;
} __attribute__((packed));

uint8_t read_pci_config_byte(int bus, int slot, int function, int offset);
uint16_t read_pci_config_word(int bus, int slot, int function, int offset);
void write_pci_config_byte(int bus, int slot, int function, size_t offset, uint8_t value);
void pci_iomemcpy(int bus, int slot, int function, const void* s, size_t l);
void pci_write_header_back(int bus, int slot, int function, pci_header_common* pci_hdr);
void pci_init();
void pci_set_command(int bus, int slot, int function, uint16_t new_cmd);
void write_pci_config_word(int bus, int slot, int function, size_t offset, uint16_t value);

#endif //KITTY_OS_CPP_PCI_HPP