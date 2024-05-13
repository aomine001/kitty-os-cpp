//
// Created by Piotr on 12.05.2024.
//

#include "driver_ctrl.hpp"

bool driver_ctrl_find_and_call(pci_handle_t pci_handle)
{
    size_t driver_array_size = (__driver_array_end - __driver_array);

    for (size_t i = 0; driver_array_size > i; i++)
    {
        __attribute__((aligned(0x10))) struct driver_entry_t driver_entry = __driver_array[i];

        if (driver_entry.is_loaded)
        {
            kstd::printf("The driver is already loaded!\n");
            continue;
        }

        for (size_t i = 0; driver_entry.requirements_count > i; i++)
        {
            auto* pci_requirement = &driver_entry.requirements[i];

            if (pci_requirement->match_requirement == PCI_REQ_VD &&
                pci_requirement->device_id == pci_handle.device_id &&
                pci_requirement->vendor_id == pci_handle.vendor_id
            )
            {
                // call it.
                driver_entry.driver_entry(&pci_handle);
                driver_entry.is_loaded = true;

                return true;
            }

            if (pci_requirement->match_requirement == PCI_REQ_CSP &&
                pci_requirement->_class == pci_handle._class &&
                pci_requirement->subclass == pci_handle.subclass &&
                pci_requirement->prog_if == pci_handle.prog_if
            )
            {
                // call it.
                driver_entry.driver_entry(&pci_handle);
                driver_entry.is_loaded = true;

                return true;
            }

            // kstd::printf("requirement: %hhd\n", pci_requirement->match_requirement);
        }
    }

    return false;
}

void driver_ctrl_enumerate_drivers()
{
    size_t driver_array_size = (__driver_array_end - __driver_array);

    kstd::printf("Drivers: ");
    for (size_t i = 0; i < driver_array_size; ++i)
    {
        struct driver_entry_t driver_entry = __driver_array[i];

        kstd::printf("Driver name: %s\nAuthor: %s\nVersion:%s\nDescription:%s\n\n", driver_entry.driver_name, driver_entry.driver_author, driver_entry.driver_version, driver_entry.driver_description);
    }
}