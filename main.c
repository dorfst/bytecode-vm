#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "parse.h"
#include "serialise.h"

int main() {

    struct line program[256];

    FILE* fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("failed to open");
        return 1;
    }
    int line_number_parsing = 0;
    int line_number = 0;

    FILE* output = fopen("out.bc", "wb");

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        struct line instruction = parse_line(line, &line_number_parsing);
        if (!instruction.is_label) {
            program[line_number] = instruction;
            ++line_number;
        }
        // printf("%d\n", instruction.opcode);
        // printf("%d\n", instruction.args[0]);
        // printf("%d\n", instruction.args[1]);
        // printf("%d\n", instruction.args[2]);

    }

    for (int i = 0; i < line_number; i++) {
        serialise_line(&program[i], output);
    }

    return 0;
}
