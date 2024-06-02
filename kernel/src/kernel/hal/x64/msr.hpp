//
// Created by Piotr on 01.06.2024.
//

#ifndef KITTY_OS_CPP_MSR_HPP
#define KITTY_OS_CPP_MSR_HPP

inline int cpu_read_msr(uint32_t reg, uint64_t *value)
{
    uint32_t low, high;

    // Inline assembly to read MSR
    __asm__ __volatile__ (
            "rdmsr" : "=a"(low), "=d"(high) : "c"(reg)
            );

    *value = ((uint64_t)high << 32) | low;
    return 0;
}

// Function to write to an MSR register
inline int cpu_write_msr(uint32_t reg, uint64_t value)
{
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);

    // Inline assembly to write MSR
    __asm__ __volatile__ (
            "wrmsr" : : "c"(reg), "a"(low), "d"(high)
            );

    return 0;
}

#endif //KITTY_OS_CPP_MSR_HPP
