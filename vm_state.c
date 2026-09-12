#include "vm_state.h"

int instruction_count(FILE* bytecode_file) {
    fseek(bytecode_file, 0, SEEK_END);
    int file_size = ftell(bytecode_file);
    fseek(bytecode_file, 0, SEEK_SET);

    // each instruction is 26 bytes long
    return file_size / 26;
}



void load_program(struct vm_state* vm_state, FILE* bytecode_file) {
    int instructions = instruction_count(bytecode_file);
    vm_state->program = malloc(sizeof(struct instruction) * instructions);
    vm_state->instruction_count = instructions;

    int current_instruction_number = 0;

    while (current_instruction_number < instructions) {
        struct instruction* instr = &vm_state->program[current_instruction_number];
        fread(&instr->opcode, sizeof(uint8_t), 1, bytecode_file);
        fread(&instr->arg_info, sizeof(uint8_t), 1, bytecode_file);
        fread(instr->args, sizeof(int64_t), 3, bytecode_file);
        current_instruction_number++;
    }
}


void init_vm_state(struct vm_state* vm_state, FILE* bytecode_file) {
    load_program(vm_state, bytecode_file);
    for (int i = 0; i < 31; ++i) {
        vm_state->gp_registers[i] = 0;
    }
    for (int i = 0; i < 256; ++i) {
        vm_state->heap_memory[i] = 0;
    }
    vm_state->pc = 0;
}

