#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../headers/vm_state.h"
#include "../headers/serialise.h"
#include "../headers/execute.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_path> <destination_path>\n", argv[0]);
        return 1;
    }

    char* source_path = argv[1];
    char* destination_path = argv[2];

    translate(source_path, destination_path);
    struct vm_state vm_state;
    FILE* bytecode_file = fopen(destination_path, "rb");
    init_vm_state(&vm_state, bytecode_file);
    execute(&vm_state);

    printf("gp registers\n");
    for (int i = 0; i < 31; ++i) {
        printf("%lu\n", vm_state.gp_registers[i]);
    }
    printf("\n\n\nheap memory\n");
    for (int i = 0; i < 256; ++i) {
        printf("%lu\n", vm_state.heap_memory[i]);
    }


    fclose(bytecode_file);

    cleanup_vm_state(&vm_state);

    return 0;
}
