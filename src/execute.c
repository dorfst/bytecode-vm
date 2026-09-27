#include "../headers/execute.h"
#include "../headers/parse.h"
#include "../headers/vm_state.h"
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
    [JMP] = jmp,
    [CMP] = cmp,
    [JGT] = jgt,
    [JLT] = jlt,
    [JEQ] = jeq,
    [JNE] = jne
};

void nop(struct vm_state* vm, struct instruction* instruction) {
    ++vm->pc;
}

void add(struct vm_state* vm, struct instruction* instruction) {
    // bits 0-2 tell you whether the number is a register number or not
    uint8_t arg_2_is_register = (instruction->arg_info >> 1) & 1;
    uint8_t arg_3_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];
    uint64_t arg3 = arg_3_is_register ? vm->gp_registers[instruction->args[2]] : instruction->args[2];

    vm->gp_registers[instruction->args[0]] = arg2 + arg3;
    ++vm->pc;
}

void sub(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = (instruction->arg_info >> 1) & 1;
    uint8_t arg_3_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];
    uint64_t arg3 = arg_3_is_register ? vm->gp_registers[instruction->args[2]] : instruction->args[2];

    vm->gp_registers[instruction->args[0]] = arg2 - arg3;
    ++vm->pc;
}

void mul(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = (instruction->arg_info >> 1) & 1;
    uint8_t arg_3_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];
    uint64_t arg3 = arg_3_is_register ? vm->gp_registers[instruction->args[2]] : instruction->args[2];

    vm->gp_registers[instruction->args[0]] = arg2 * arg3;
    ++vm->pc;
}

void divide(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = (instruction->arg_info >> 1) & 1;
    uint8_t arg_3_is_register = instruction->arg_info & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];
    uint64_t arg3 = arg_3_is_register ? vm->gp_registers[instruction->args[2]] : instruction->args[2];

    vm->gp_registers[instruction->args[0]] = (uint64_t)(arg2 / arg3);
    ++vm->pc;
}

void mov(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = (instruction->arg_info >> 1) & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];

    vm->gp_registers[instruction->args[0]] = arg2;
    ++vm->pc;
}

void ldr(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_2_is_register = (instruction->arg_info >> 1) & 1;

    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];

    vm->gp_registers[instruction->args[0]] = vm->heap_memory[arg2];
    ++vm->pc;
}

void str(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_1_is_register = (instruction->arg_info >> 2) & 1;
    uint8_t arg_2_is_register = (instruction->arg_info >> 1) & 1;

    uint64_t arg1 = arg_1_is_register ? vm->gp_registers[instruction->args[0]] : instruction->args[0];
    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];

    vm->heap_memory[arg1] = arg2;
    ++vm->pc;
}

void jmp(struct vm_state* vm, struct instruction* instruction) {
    vm->pc = instruction->args[0];
}

void cmp(struct vm_state* vm, struct instruction* instruction) {
    uint8_t arg_1_is_register = (instruction->arg_info >> 2) & 1;
    uint8_t arg_2_is_register = (instruction->arg_info >> 1) & 1;

    uint64_t arg1 = arg_1_is_register ? vm->gp_registers[instruction->args[0]] : instruction->args[0];
    uint64_t arg2 = arg_2_is_register ? vm->gp_registers[instruction->args[1]] : instruction->args[1];

    vm->comparison_flags[0] = false;
    vm->comparison_flags[1] = false;
    vm->comparison_flags[2] = false;

    if (arg1 == arg2) vm->comparison_flags[0] = true;
    else if (arg1 > arg2) {
        vm->comparison_flags[0] = false;
        vm->comparison_flags[1] = true;
    }
    else if (arg1 < arg2) {
        vm->comparison_flags[0] = false;
        vm->comparison_flags[2] = true;
    }

    ++vm->pc;
}

void jgt(struct vm_state* vm, struct instruction* instruction) {
    if (vm->comparison_flags[1] == true) {
        vm->pc = instruction->args[0];
        reset_comparison_flags(vm);
    } else {
        ++vm->pc;
    }
}

void jlt(struct vm_state* vm, struct instruction* instruction) {
    if (vm->comparison_flags[2] == true) {
        vm->pc = instruction->args[0];
        reset_comparison_flags(vm);
    } else {
        ++vm->pc;
    }

}

void jeq(struct vm_state* vm, struct instruction* instruction) {
    if (vm->comparison_flags[0] == true) {
        vm->pc = instruction->args[0];
        reset_comparison_flags(vm);
    } else {
        ++vm->pc;
    }
}

void jne(struct vm_state* vm, struct instruction* instruction) {
    if (vm->comparison_flags[0] == false) {
        vm->pc = instruction->args[0];
        reset_comparison_flags(vm);
    } else {
        ++vm->pc;
    }
}

void execute(struct vm_state* vm) {
    while (vm->pc < vm->instruction_count) {
        struct instruction current_instruction = vm->program[vm->pc];
        uint8_t opcode = current_instruction.opcode;
        if (dispatch_table[opcode] == NULL) {
            printf("unknown opcode %d\n", opcode);
            return;
        }
        dispatch_table[opcode](vm, &current_instruction);
    }
}