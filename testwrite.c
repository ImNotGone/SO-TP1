#include "ADTs/shm.h"

#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
    if(argc != 2) return 1;
    destroy_shm("writer");
    char * buff = map_shm("writer", BLOCK_SIZE, true);
    if(buff == NULL) {
        printf("ERROR MAPPING");
        return -1;
    }
    printf("[WRITING] %s\n", argv[1]);
    // strncpy(buff, argv[1], BLOCK_SIZE);
    write_shm(buff, argv[1], strlen(argv[1]) + 1);
    unmap_shm("writer", BLOCK_SIZE);
    return 0;
}
