#include "../include/idt.hpp"

namespace onx
{
    static constexpr size_t IDT_ENTRIES = 256;
    static IdtEntry idt[IDT_ENTRIES];
    static Idtr idtr;

    static constexpr uint8_t make_type_attr(GateType type, Dpl dpl, bool present = true) 
    {
        return (present ? 0x80 : 0x00) | 
               (static_cast<uint8_t>(dpl) << 5) | 
               static_cast<uint8_t>(type);
    }

    void set_gate(Vector vector, void* handler, GateType type, Dpl dpl, uint8_t ist) 
    {
        uint64_t addr = reinterpret_cast<uint64_t>(handler);
        idt[static_cast<uint8_t>(vector)].offset_low = addr & 0xFFFF;
        idt[static_cast<uint8_t>(vector)].selector = 0x28; // kernel code from limine; may have to pass into as void*
        idt[static_cast<uint8_t>(vector)].ist = ist;
        idt[static_cast<uint8_t>(vector)].type_attr = make_type_attr(type, dpl);
        idt[static_cast<uint8_t>(vector)].offset_mid = (addr >> 16) & 0xFFFF;
        idt[static_cast<uint8_t>(vector)].offset_high = (addr >> 32) & 0xFFFFFFFF;
        idt[static_cast<uint8_t>(vector)].reserved = 0;
    }

    void init_idt() 
    {
        idtr.limit = sizeof(IdtEntry) * IDT_ENTRIES - 1;
        idtr.base = reinterpret_cast<uint64_t>(&idt[0]);
        for (auto& entry : idt)
            entry = {};

        asm volatile("lidt %0" : : "m"(idtr));
    }
}