#include "../headers/parse.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct hash_table_node* symbol_table[256];
const char* instructions[] = {"NOP", "ADD", "SUB", "MUL", "DIV", "MOV", "LDR",
    "STR", "JMP", "CMP", "JGT", "JLT", "JEQ", "JNE"};

// hashing for resolving labels
unsigned long djb2(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        // hash = hash * 33 (32 + 1) + c
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int search_linked_list(struct hash_table_node* head, char* label) {
    if (head == NULL) {
        // error
        return -1;
    }
    struct hash_table_node* current = head;
    while (current != NULL) {
        if (strcmp(current->label, label) == 0) {
            return current->line_number;
        }
        current = current->next;
    }
    // error
    return -1;
}

int insert_into_linked_list(struct hash_table_node* head, struct hash_table_node* node_to_insert) {
    if (head == NULL) {
        // invalid head
        return 1;
    }
    struct hash_table_node* current = head;
    while (current->next != NULL) {
        if (strcmp(current->label, node_to_insert->label) == 0) {
            // duplicate
            return -1;
        }
        current = current->next;
    }

    current->next = node_to_insert;
    return 0;
}

int insert_into_hash_table(struct hash_table_node* hash_table[], struct hash_table_node* node_to_insert) {
    if (hash_table == NULL) {
        return -1;
    }
    int index = djb2(node_to_insert->label) % 256;
    if (hash_table[index] == NULL) {
        hash_table[index] = node_to_insert;
        return 0;
    }
    int result = insert_into_linked_list(hash_table[index], node_to_insert);
    return result;

}

int search_hash_table(struct hash_table_node* hash_table[], char* label) {
    if (hash_table == NULL) {
        return -1;
    }
    int index = djb2(label) % 256;
    if (hash_table[index] == NULL) {
        return -1;
    }
    int result = search_linked_list(hash_table[index], label);
    return result;

}

int match_to_opcode(char* string) {
    for (uint64_t i = 0; i < sizeof(instructions) / sizeof(instructions[0]); ++i) {
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
// how many operands are required for each opcode
int operands_required(int opcode) {
    if (opcode == 0) return 0;
    if (opcode == 1) return 3;
    if (opcode == 2) return 3;
    if (opcode == 3) return 3;
    if (opcode == 4) return 3;
    if (opcode == 5) return 2;
    if (opcode == 6) return 2;
    if (opcode == 7) return 2;
    if (opcode == 8) return 1;
    if (opcode == 9) return 2;
    if (opcode == 10) return 1;
    if (opcode == 11) return 1;
    if (opcode == 12) return 1;
    if (opcode == 13) return 1;
    return -1;
}

// extract number from a larger string, start_index must be where the first numeric character is
uint64_t get_number(char* buffer, int start_index) {
    char current_char = buffer[start_index];
    int current_index = start_index;

    while (current_char >= 48 && current_char <= 57) {
        ++current_index;
        current_char = buffer[current_index];
    }

    // current_index will overshoot by 1
    char* end = buffer + current_index - 1;

    uint64_t number = strtoull(buffer + start_index, &end, 10);

    return number;
}

void get_operands(char* line, int opcode, struct line* line_struct) {
    int num_operands = operands_required(opcode);
    line_struct->num_args = num_operands;
    int line_index = 0;
    char current_char = line[line_index];
    int operand_number = 0;

    while (current_char != '\0' && operand_number < num_operands) {
        current_char = line[line_index];
        if (current_char == 'r') {
            // get number expects first character to be numeric
            uint64_t reg_number = get_number(line, line_index + 1);
            line_struct->args[operand_number] = reg_number;
            line_struct->is_register[operand_number] = true;
            ++operand_number;
        }
        if (current_char == '#') {
            uint64_t number = get_number(line, line_index + 1);
            line_struct->args[operand_number] = number;
            line_struct->is_register[operand_number] = false;
            ++operand_number;
        }
        ++line_index;
    }

    for (int i = 3; i > num_operands; --i) {
        line_struct->args[i - 1] = 0;
        line_struct->is_register[i - 1] = false;
    }
}

void get_label(char* line, struct line* line_struct) {
    // "jmp "
    const int offset = strcspn(line, " ") + 1;
    char* start = line + offset;
    size_t len = strcspn(start, "\n");
    line_struct->label = (char*)malloc(sizeof(char) * (len + 1));
    strncpy(line_struct->label, start, len);
    line_struct->label[len] = '\0';
}

int get_opcode(char* line) {
    char* instruction = get_instruction(line);
    int opcode = match_to_opcode(instruction);
    free(instruction);
    return opcode;
}

struct line parse_line(char* buffer, int* line_number) {
    struct line line;
    line.is_label = false;
    line.is_comment = false;
    // note that this means that you cannot have comments on the same line as an instruction
    if (strchr(buffer, ';') != NULL) {
        line.is_comment = true;
        return line;
    }
    // is a label
    if (strchr(buffer, ':') != NULL) {
        line.is_label = true;
        char* colon = strchr(buffer, ':');
        size_t length = colon - buffer;
        char* label = (char*)malloc(sizeof(char) * (length + 1));
        strncpy(label, buffer, length);
        label[length] = '\0';
        struct hash_table_node* label_node = (struct hash_table_node*)malloc(sizeof(struct hash_table_node));
        label_node->label = label;
        label_node->line_number = *line_number;
        label_node->next = NULL;
        insert_into_hash_table(symbol_table, label_node);
        return line;
    }
    int opcode = get_opcode(buffer);
    line.opcode = opcode;
    if (opcode != 8 && opcode < 10) {
        get_operands(buffer, opcode, &line);
    }
    else if (opcode == 8 || opcode >= 10) {
        line.num_args = 1;
        line.args[0] = 0;
        line.args[1] = 0;
        line.args[2] = 0;

        line.is_register[0] = false;
        line.is_register[1] = false;
        line.is_register[2] = false;

        get_label(buffer, &line);
    }
    (*line_number)++;
    return line;
}

