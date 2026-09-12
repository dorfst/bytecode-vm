#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "vm_state.h"

int main() {

    struct vm_state vm_state;
    FILE* bytecode_file = fopen("out.bc", "rb");
    init_vm_state(&vm_state, bytecode_file);

    printf("%d", vm_state.instruction_count);

    return 0;
}
