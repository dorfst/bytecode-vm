//
// Created by dorian on 23/08/2026.
//

#ifndef PARSE
#define PARSE
#include <stdint.h>
#include <stdbool.h>

struct line {
    bool is_label;
    // used for jmp instruction as well as lines that are labels
    char* label;
    uint8_t opcode;
    uint64_t args[3];
    uint8_t num_args;
    bool is_register[3];
};

struct hash_table_node {
    // make sure it matches when collisions occur
    char* label;
    int line_number;
    struct hash_table_node* next;
};
unsigned long djb2(const char* str);
int search_linked_list(struct hash_table_node* head, char* label);
int insert_into_linked_list(struct hash_table_node* head, struct hash_table_node* node_to_insert);
int insert_into_hash_table(struct hash_table_node hash_table[], struct hash_table_node* node_to_insert);
int search_hash_table(struct hash_table_node hash_table[], char* label);
int match_to_opcode(char* string);
char* get_instruction(char* buffer);
int operands_required(int opcode);
uint64_t get_number(char* buffer, int start_index);
void get_operands(char* line, int opcode, struct line* line_struct);
int get_opcode(char* line);
struct line parse_line(char* buffer, int* line_number);
#endif
