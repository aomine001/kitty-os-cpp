bits 64

section .text

extern interrupt_handler

global flush_idt_asm
flush_idt_asm:
    lidt [rdi]
    ret

%macro ISR_NOERRORCODE 1
    global idt_isr%1:
    idt_isr%1:
        push qword 0 ; dummy error code
        push qword %1 ; interrupt number
        jmp isr_common
%endmacro

%macro ISR_ERRORCODE 1
    global idt_isr%1:
    idt_isr%1:
        push qword %1 ; interrupt number
        jmp isr_common
%endmacro

%include 'src/kernel/hal/x64/idt/isr_expansions.inc'

isr_common:
    swapgs
    push rax
    push rbx
    push rcx
    push rdx
    push rsp
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    cld
    call interrupt_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rsp
    pop rdx
    pop rcx
    pop rbx
    pop rax

    swapgs

    add rsp, 16
    iretq


    call interrupt_handler
    add rsp, 16
    iretq

global enable_interrupts
enable_interrupts:
    sti
    ret

global disable_interrupts
disable_interrupts:
    cli
    ret