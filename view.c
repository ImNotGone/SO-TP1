// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
#include "ADTs/pshmADT.h"
#include "ADTs/smADT.h"
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024
#define SCAN_FORMAT_SIZE 16
#define MODE 0666

static void failNExit(const char *msg);

int main(int argc, char *argv[]) {

    pshmADT pshm;
    int fileAmount;

    // Parse arguments
    if (argc == 1) {
        // Read arguments from stdin ($> pshmName semName fileAmount)
        char pshmName[BUFFER_SIZE];
        char semName[BUFFER_SIZE];
        char scanFormat[SCAN_FORMAT_SIZE];

        // scanf "%(BUFFER_SIZE)s %(BUFFER_SIZE)s %d"
        sprintf(scanFormat, "%%%ds %%%ds %%d", BUFFER_SIZE, BUFFER_SIZE);
        printf("scanFormat: %s\n", scanFormat);
        if (scanf(scanFormat, pshmName, semName, &fileAmount) != 3) {
            errno = EINVAL;
            failNExit("Invalid stdin arguments");
        }

        pshm = newPshm(pshmName, semName, O_RDWR, MODE);

    } else if (argc == 4) {
        // Get arguments from argv
        pshm = newPshm(argv[1], argv[2], O_RDWR, MODE);
        fileAmount = strtol(argv[3], NULL, 10);

    } else {
        errno = EINVAL;
        failNExit("Invalid arguments");
    }

    // Validate shared memory is vaild
    if (pshm == NULL) {
        failNExit("Error creating pshm");
    }

    // Read shared memory once per file
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

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
