#include <oronyx/arch/cpu.hpp>

/* cxxrt support */
extern "C"
{
    // @note may add destructor tracking
    int __cxa_atexit(void (*)(void *), void *, void *) 
    { 
        return 0; 
    }

    void __cxa_pure_virtual() 
    { 
        onx::cpu::halt(); 
    }
    
    void *__dso_handle;
}