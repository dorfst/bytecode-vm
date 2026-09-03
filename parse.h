//
// Created by dorian on 23/08/2026.
//

#ifndef BYTECODE_VM_PARSE_H
#define BYTECODE_VM_PARSE_H
#include <stdint.h>
int match_to_opcode(char* string);
char* get_instruction(char* buffer);
uint8_t get_operand(char* line, int operand_number);
int get_opcode(char* line);
#endif //BYTECODE_VM_PARSE_H
