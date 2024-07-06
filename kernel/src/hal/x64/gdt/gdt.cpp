//
// Created by Piotr on 30.04.2024.
//

#include <hal/x64/gdt/gdt.hpp>

struct GDTSegmentDescriptor gdt[] = {
    // NULL
    GDT_ENTRY(0, 0, 0, 0),

    // Kernel Code & data
    GDT_ENTRY(0, 0, GDT_PRESENT | GDT_DPL0 | GDT_EXECUTABLE_BIT | GDT_DESCRIPTOR_TYPE_BIT | GDT_READ_WRITE_BIT, GDT_LONG_MODE_FLAG | GDT_GRANULARITY_FLAG),
    GDT_ENTRY(0, 0, GDT_PRESENT | GDT_DPL0 | GDT_DESCRIPTOR_TYPE_BIT | GDT_READ_WRITE_BIT, GDT_SIZE_FLAG | GDT_GRANULARITY_FLAG),

    // Drivers Code & data
    GDT_ENTRY(0, 0, GDT_PRESENT | GDT_DPL1 | GDT_EXECUTABLE_BIT | GDT_DESCRIPTOR_TYPE_BIT | GDT_READ_WRITE_BIT, GDT_LONG_MODE_FLAG | GDT_GRANULARITY_FLAG),
    GDT_ENTRY(0, 0, GDT_PRESENT | GDT_DPL1 | GDT_DESCRIPTOR_TYPE_BIT | GDT_READ_WRITE_BIT, GDT_SIZE_FLAG | GDT_GRANULARITY_FLAG),

    // Program supervisors Code & Data
    GDT_ENTRY(0, 0, GDT_PRESENT | GDT_DPL2 | GDT_EXECUTABLE_BIT | GDT_DESCRIPTOR_TYPE_BIT | GDT_READ_WRITE_BIT, GDT_LONG_MODE_FLAG | GDT_GRANULARITY_FLAG),
    GDT_ENTRY(0, 0, GDT_PRESENT | GDT_DPL2 | GDT_DESCRIPTOR_TYPE_BIT | GDT_READ_WRITE_BIT, GDT_SIZE_FLAG | GDT_GRANULARITY_FLAG),

    // Kernel code & data
    GDT_ENTRY(0, 0, GDT_PRESENT | GDT_DPL3 | GDT_EXECUTABLE_BIT | GDT_DESCRIPTOR_TYPE_BIT | GDT_READ_WRITE_BIT, GDT_LONG_MODE_FLAG | GDT_GRANULARITY_FLAG),
    GDT_ENTRY(0, 0, GDT_PRESENT | GDT_DPL3 | GDT_DESCRIPTOR_TYPE_BIT | GDT_READ_WRITE_BIT, GDT_SIZE_FLAG | GDT_GRANULARITY_FLAG),

    // TSS space
    GDT_ENTRY(0,0,0,0),
    GDT_ENTRY(0,0,0,0),
};

void flush_gdt()
{
    struct GDTR gdtr = {
        .size = sizeof(gdt) - 1,
        .gdt_address = reinterpret_cast<uint64_t>(&gdt)
    };

    flush_gdt_asm(&gdtr);
}