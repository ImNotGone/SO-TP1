// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "ADTs/pshmADT.h"

#include <stdio.h>
#include <string.h>

/*
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
*/
int main (int argc, char *argv[]) {
    if(argc != 1) return 1;
    pshmADT pshm = newPshm("writer", O_RDWR, PROT_WRITE|PROT_READ);
    // printf("[READ] %s\n", buff);

    char aux_buff[1024];
    readPshm(pshm, aux_buff, 1024 - 1);
    printf("[READ] %s\n", aux_buff);

    sleep(6);
    freePshm(pshm);
    return 0;
}
