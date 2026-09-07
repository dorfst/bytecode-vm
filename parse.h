//
// Created by dorian on 23/08/2026.
//

#ifndef PARSE
#define PARSE
#include <stdint.h>
#include <stdbool.h>

struct line {
    bool is_label;
    char *label;
    int opcode;
    uint64_t args[3];
    bool is_register[3];
};

int match_to_opcode(char* string);
char* get_instruction(char* buffer);
uint8_t get_operand(char* line, int operand_number);
int get_opcode(char* line);
struct line parse_line(char* buffer);
#endif
