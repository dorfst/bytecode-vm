#ifndef VM_STATE
#define VM_STATE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

struct instruction {
    uint8_t opcode;
    uint8_t arg_info;
    uint64_t args[3];
};

struct vm_state {
    uint64_t gp_registers[31];
    uint64_t heap_memory[256];
    uint64_t pc;
    int instruction_count;
    struct instruction* program;
};

int instruction_count(FILE* bytecode_file);
void load_program(struct vm_state* vm_state, FILE* bytecode_file);
void init_vm_state(struct vm_state* vm_state, FILE* bytecode_file);

#endif
