#include "execute.h"
#include "parse.h"
#include "vm_state.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



instruction_handler dispatch_table[NUM_OPCODES] = {
    [NOP] = nop,
    [ADD] = add,
    [SUB] = sub,
    [MUL] = mul,
    [DIV] = divide,
    [MOV] = mov,
    [LDR] = ldr,
    [STR] = str,
    [JMP] = jmp
};

void nop(struct vm_state* vm, struct instruction* instruction) {
    ++vm->pc;
}

void add(struct vm_state* vm, struct instruction* instruction) {
    // bits 0-2 tell you whether the number is a register number or not
    printf("add instruction running");
    uint8_t arg_2_is_register = instruction->arg_info & 2;
    uint8_t arg_3_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];
    uint64_t arg3 = arg_3_is_register ? vm->gp_registers[instruction->args[2]] : instruction->args[2];

    vm->gp_registers[instruction->args[0]] = arg2 + arg3;
    ++vm->pc;
}

void sub(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = instruction->arg_info & 2;
    uint8_t arg_3_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];
    uint64_t arg3 = arg_3_is_register ? vm->gp_registers[instruction->args[2]] : instruction->args[2];

    vm->gp_registers[instruction->args[0]] = arg2 - arg3;
    ++vm->pc;
}

void mul(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = instruction->arg_info & 2;
    uint8_t arg_3_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];
    uint64_t arg3 = arg_3_is_register ? vm->gp_registers[instruction->args[2]] : instruction->args[2];

    vm->gp_registers[instruction->args[0]] = arg2 * arg3;
    ++vm->pc;
}

void divide(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = instruction->arg_info & 2;
    uint8_t arg_3_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];
    uint64_t arg3 = arg_3_is_register ? vm->gp_registers[instruction->args[2]] : instruction->args[2];

    vm->gp_registers[instruction->args[0]] = (uint64_t)(arg2 / arg3);
    ++vm->pc;
}

void mov(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];

    vm->gp_registers[instruction->args[0]] = arg2;
    ++vm->pc;
}

void ldr(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];

    vm->gp_registers[instruction->args[0]] = vm->heap_memory[arg2];
    ++vm->pc;
}

void str(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_1_is_register = instruction->arg_info & 1;
    uint8_t arg_2_is_register = instruction->arg_info & 2;

    uint64_t arg1 = arg_1_is_register ? vm->gp_registers[instruction->args[0]] : instruction->args[0];
    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];

    vm->heap_memory[arg1] = vm->gp_registers[arg2];
    ++vm->pc;
}

void jmp(struct vm_state* vm, struct instruction* instruction) {
    vm->pc = instruction->args[0];
}

void execute(struct vm_state* vm) {
    while (vm->pc < vm->instruction_count) {
        struct instruction current_instruction = vm->program[vm->pc];
        uint8_t opcode = current_instruction.opcode;
        printf("about to dispatch: pc=%llu opcode=%d handler=%p\n", vm->pc, opcode, (void*)dispatch_table[opcode]);
        if (dispatch_table[opcode] == NULL) {
            printf("unknown opcode %d\n", opcode);
            return;
        }
        dispatch_table[opcode](vm, &current_instruction);
    }
}