//
// Created by Piotr on 05.05.2024.
//

#include "sb16.hpp"

bool is_sb16_available()
{
    outb(DSP_RESET, 1);
    iowait(1);
    outb(DSP_RESET, 0);
    iowait(1);

    if (inb(DSP_READ) != 0xAA)
    {
        return false;
    }

    return true;
}

void dsp_write(kstd::uint8_t value)
{
    kstd::size_t status;

    status = static_cast<kstd::size_t>(inb(DSP_WRITE));

    outb(DSP_WRITE, value);
}

kstd::uint8_t dsp_read()
{
    return inb(DSP_READ);
}
