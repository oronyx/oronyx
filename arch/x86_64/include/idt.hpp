#pragma once
#include <types.hpp>

namespace onx
{
    enum class Vector : uint8_t 
    {
        DIVIDE_ERROR = 0x0,
        DEBUG = 0x1,
        NMI = 0x2,
        BREAKPOINT = 0x3,
        OVERFLOW = 0x4,
        BOUND_RANGE = 0x5,
        INVALID_OPCODE = 0x6,
        DEVICE_NOT_AVAILABLE = 0x7,
        DOUBLE_FAULT = 0x8,
        COPROCESSOR_SEGMENT = 0x9,
        INVALID_TSS = 0xA,
        SEGMENT_NOT_PRESENT = 0xB,
        STACK_FAULT = 0xC,
        GENERAL_PROTECTION = 0xD,
        PAGE_FAULT = 0xE,
        RESERVED_15 = 0xF,
        X87_FPU = 0x10,
        ALIGNMENT_CHECK = 0x11,
        MACHINE_CHECK = 0x12,
        SIMD_FPU = 0x13,
        VIRTUALIZATION = 0x14,
        CONTROL_PROTECTION = 0x15,

        // 22-27 reserved

        HYPERVISOR = 28,
        VMM_COMMUNICATION = 29,
        SECURITY_EXCEPTION = 30,
        
        // 31 reserved
        
        // IRQs
        TIMER = 32,
        KEYBOARD = 33,
        CASCADE = 34,
        COM2 = 35,
        COM1 = 36,
        LPT2 = 37,
        FLOPPY = 38,
        LPT1 = 39,
        CMOS_RTC = 40,
        PERIPHERAL1 = 41,
        PERIPHERAL2 = 42,
        PERIPHERAL3 = 43,
        MOUSE = 44,
        FPU = 45,
        PRIMARY_ATA = 46,
        SECONDARY_ATA = 47,

        SYSCALL = 0x80,
    };

    enum class GateType : uint8_t
    {
        INTERRUPT = 0xE,
        TRAP = 0xF
    };

    enum class Dpl : uint8_t
    {
        KERNEL = 0x0,
        USER = 0x3
    };

    struct [[gnu::packed]] IdtEntry
    {
        uint16_t offset_low;
        uint16_t selector;
        uint8_t  ist;
        uint8_t  type_attr;
        uint16_t offset_mid;
        uint32_t offset_high;
        uint32_t reserved;
    };

    struct [[gnu::packed]] Idtr
    {
        uint16_t limit;
        uint64_t base;
    };

    void set_gate(Vector vector, void* handler, GateType type, Dpl dpl, uint8_t ist = 0);

    void init_idt();
}