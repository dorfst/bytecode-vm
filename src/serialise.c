#include "../headers/parse.h"
#include "../headers/serialise.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const char* instructions[];
extern struct hash_table_node* symbol_table[];

int serialise_line(struct line* line, FILE* fp) {
    if (line == NULL) return -1;
    if (!line->is_label && !line->is_comment) {
        uint8_t opcode = line->opcode;
        uint8_t registers_byte = (line->is_register[0] << 2) | (line->is_register[1] << 1)
                                | (line->is_register[2]);
        uint8_t num_args = line->num_args << 3;
        uint8_t arg_info = registers_byte | num_args;


        // not any sort of jump instruction
        if (opcode != 8 && opcode < 10) {
            fwrite(&opcode, sizeof(uint8_t), 1, fp);
            fwrite(&arg_info, sizeof(uint8_t), 1, fp);
            fwrite(&line->args[0], sizeof(uint64_t), 1, fp);
            fwrite(&line->args[1], sizeof(uint64_t), 1, fp);
            fwrite(&line->args[2], sizeof(uint64_t), 1, fp);
            return 0;
        }
        else {
            fwrite(&opcode, sizeof(uint8_t), 1, fp);

            uint8_t one_arg = 1 << 3;
            fwrite(&one_arg, sizeof(uint8_t), 1, fp);

            int line_to_jump_to = search_hash_table(symbol_table, line->label);
            if (line_to_jump_to == -1) {
                fprintf(stderr, "failed to find label %s\n", line->label);
                exit(1);
            }
            uint64_t line_jump_to = (uint64_t)line_to_jump_to;
            fwrite(&line_jump_to, sizeof(uint64_t), 1, fp);

            uint64_t zero_uint64 = 0;
            fwrite(&zero_uint64, sizeof(uint64_t), 1, fp);
            fwrite(&zero_uint64, sizeof(uint64_t), 1, fp);


            free(line->label);
            return 0;
        }
    }
    return -1;
}

void cleanup_symbol_table(struct hash_table_node* symbol_table[]) {
    for (int i = 0; i < 256; ++i) {
        struct hash_table_node* current = symbol_table[i];
        while (current != NULL) {
            free(current->label);
            struct hash_table_node* old = current;
            current = current->next;
            free(old);
        }
    }
}

void translate(const char* source, const char* out) {
    struct line program[256];

    FILE* fp = fopen(source, "r");
    if (fp == NULL) {
        perror("failed to open source");
        exit(1);
    }
    int line_number_parsing = 0;
    int line_number = 0;

    FILE* output = fopen(out, "wb");
    if (output == NULL) {
        perror("failed to open output");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        struct line instruction = parse_line(line, &line_number_parsing);
        if (!instruction.is_label && !instruction.is_comment) {
            program[line_number] = instruction;
            ++line_number;
        }
        if (line_number >= 256) {
            fprintf(stderr, "program too long\n");
            exit(1);
        }
    }

    for (int i = 0; i < line_number; i++) {
        serialise_line(&program[i], output);
    }

    cleanup_symbol_table(symbol_table);

    fclose(fp);
    fclose(output);
}