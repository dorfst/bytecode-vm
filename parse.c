#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const char* instructions[] = {"NOP", "ADD", "SUB", "MUL", "DIV", "MOV", "LDR", "STR"};

int match_to_opcode(char* string) {
    for (int i = 0; i < sizeof(instructions) / sizeof(instructions[0]); ++i) {
        if (strcmp(string, instructions[i]) == 0) {
            return i;
        }
    }
    return -1;
}

// instruction is always first, so just keep on going until first space
char* get_instruction(char* buffer) {
    int num_chars_in_instruction = 0;
    int current_char = 0;

    while (buffer[current_char] != '\0') {
        if (buffer[current_char] == ' ') {
            break;
        }
        ++current_char;
        ++num_chars_in_instruction;
    }

    char* instruction_buffer = malloc(sizeof(char) * (num_chars_in_instruction + 1));

    for (int i = 0; i < num_chars_in_instruction; ++i) {
        instruction_buffer[i] = buffer[i];
    }

    // don't forget null character at the end
    instruction_buffer[num_chars_in_instruction] = '\0';

    return instruction_buffer;
}

// valid operands are r0-r9 or #0 to #255
uint8_t get_operand(char* line, int operand_number) {
    int i = 0;
    int operands_encountered = 0;
    char current_char = line[i];
    while (current_char != '\0') {
        current_char = line[i];
        if (current_char == 'r') {
            ++operands_encountered;
            if (operands_encountered == operand_number) {
                char next_char = line[i + 1];
                int register_number = atoi(&next_char);
                // 1 bit for register flag, 7 bits for register identifier or immediate value
                uint8_t operand = 0b10000000 + register_number;
                return operand;
            }
        }
        else if (current_char == '#') {
            ++operands_encountered;
            char str_operand[3];
            if (operands_encountered == operand_number) {
                // move forward to beginning of immediate value
                ++i;
                int beginning_of_operand = i;
                current_char = line[beginning_of_operand];
                int number_of_digits = 0;
                // get digits after #
                while (current_char >= 48 && current_char <= 57) {
                    ++number_of_digits;
                    ++i;
                    current_char = line[i];
                }
                // form number into string
                for (int j = beginning_of_operand; j < beginning_of_operand + number_of_digits; ++j) {
                    strncat(str_operand, &line[j], 1);
                }
                uint8_t operand = atoi(str_operand);
                return operand;
            }
        }
        ++i;
    }
}

int get_opcode(char* line) {
    char* instruction = get_instruction(line);
    int opcode = match_to_opcode(instruction);
    return opcode;
}