#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "vm_state.h"
#include "serialise.h"
#include "execute.h"

int main() {
    translate("file.txt", "out.bc");
    struct vm_state vm_state;
    FILE* bytecode_file = fopen("out.bc", "rb");
    init_vm_state(&vm_state, bytecode_file);
    execute(&vm_state);

    printf("gp registers\n");
    for (int i = 0; i < 31; ++i) {
        printf("%d\n", vm_state.gp_registers[i]);
    }
    printf("\n\n\nheap memory\n");
    for (int i = 0; i < 256; ++i) {
        printf("%d\n", vm_state.heap_memory[i]);
    }


    fclose(bytecode_file);

    return 0;
}
