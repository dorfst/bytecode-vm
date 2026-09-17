

#ifndef EXECUTE
#define EXECUTE
#include <stdio.h>
#include "vm_state.h"
#define NOP 0
#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4
#define MOV 5
#define LDR 6
#define STR 7
#define JMP 8
#define CMP 9
#define JGT 10
#define JLT 11
#define JEQ 12
#define JNE 13
#define NUM_OPCODES 14

// array of function pointers
typedef void (*instruction_handler)(struct vm_state* vm, struct instruction* instruction);

extern instruction_handler dispatch_table[NUM_OPCODES];

void nop(struct vm_state* vm, struct instruction* instruction);
void add(struct vm_state* vm, struct instruction* instruction);
void sub(struct vm_state* vm, struct instruction* instruction);
void mul(struct vm_state* vm, struct instruction* instruction);
void divide(struct vm_state* vm, struct instruction* instruction);
void mov(struct vm_state* vm, struct instruction* instruction);
void ldr(struct vm_state* vm, struct instruction* instruction);
void str(struct vm_state* vm, struct instruction* instruction);
void jmp(struct vm_state* vm, struct instruction* instruction);
void cmp(struct vm_state* vm, struct instruction* instruction);
void jgt(struct vm_state* vm, struct instruction* instruction);
void jlt(struct vm_state* vm, struct instruction* instruction);
void jeq(struct vm_state* vm, struct instruction* instruction);
void jne(struct vm_state* vm, struct instruction* instruction);
void execute(struct vm_state* vm);


#endif
