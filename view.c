// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "ADTs/pshmADT.h"
#include "ADTs/smADT.h"

#define BUFFER_SIZE 1024

static void failNExit(const char *msg);

int main (int argc, char *argv[]) {

    // Validate arguments
    if (argc != 3) {
        fprintf(stderr, "Wrong view usage\n");
        exit(EXIT_FAILURE);
    } 

    pshmADT pshm = newPshm(argv[1], argv[2], O_RDWR, 0666);
    if (pshm == NULL) {
        failNExit("Error creating pshm");
    }

    int fileAmount = strtol(argv[3], NULL, 10);

    char buffer[BUFFER_SIZE] = {0};

    for (int i = 0; i < fileAmount; i++) {

        int bytesRead = readPshm(pshm, buffer, BUFFER_SIZE);
        if (bytesRead == -1) {
            failNExit("Error reading pshm");
        }

        printf("%s\n", buffer);
    } 

    freePshm(pshm);
    return 0;
}


static void failNExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
