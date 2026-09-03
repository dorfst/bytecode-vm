#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "parse.h"

int main() {
    char* string = "ADD r1 r2 #32";

    int opcode = get_opcode(string);
    uint8_t operand1 = get_operand(string, 1);
    uint8_t operand2 = get_operand(string, 2);
    uint8_t operand3 = get_operand(string, 3);

    printf("%d\n", opcode);
    printf("%d\n", operand1);
    printf("%d\n", operand2);
    printf("%d\n", operand3);

    return 0;
}
