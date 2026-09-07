#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "parse.h"

int main() {

    FILE* fp = fopen("file.txt", "r");
    if (fp == NULL) {
        perror("failed to open");
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        struct line instruction = parse_line(line);

        printf("%d\n", instruction.opcode);
        printf("%d\n", instruction.args[0]);
        printf("%d\n", instruction.args[1]);
        printf("%d\n", instruction.args[2]);

    }

    return 0;
}
