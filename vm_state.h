//
// Created by dorian on 03/09/2026.
//

#ifndef VM_STATE
#define VM_STATE
#include <stdint.h>
struct vm_state {
    uint64_t gp_registers[31];
    uint64_t heap_memory[256];
    uint64_t pc;
    char** code_memory;
};
#endif
