//
// Created by Piotr on 03.06.2024.
//

#include <kernel/irqs/pic/pic.hpp>
#include "uniirq.hpp"

bool using_apic = false;

void uirq_mask_irq(int irq)
{
    if (using_apic) return;

    irq_set_mask(irq);
}
void uirq_unmask_irq(int irq)
{
    if (using_apic) return;

    irq_clear_mask(irq);
}

void uirq_register_irq(int irq, idt_function_pointer fn)
{
    idt_attach_interrupt(irq + 0x90, fn);
}

void uniirq_init()
{
    if (!using_apic) pic_remap(0x90, 0x98);
}