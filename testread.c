#include "ADTs/shm.h"

#include <stdio.h>
#include <string.h>

int main (int argc, char *argv[]) {
    if(argc != 1) return 1;
    char * buff = map_shm("writer", BLOCK_SIZE, false);

    // printf("[READ] %s\n", buff);

    char aux_buff[BLOCK_SIZE];
    read_shm(buff, aux_buff, BLOCK_SIZE - 1);
    printf("[READ] %s\n", aux_buff);
    unmap_shm("writer", BLOCK_SIZE);
    return 0;
}
