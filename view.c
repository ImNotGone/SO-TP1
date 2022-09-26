// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "view.h"
#include "ADTs/pshmADT.h"

#define SCAN_FORMAT "%%%ds %%d"
#define SCAN_FORMAT_SIZE 16

static void failNExit(const char *msg);

int main(int argc, char *argv[]) {


    int fileQty = 0;

    int fifoFD = -1;

    // Parse arguments
    if (argc == 1) {
        // Read arguments from stdin ($> pshmName fileQty)
        char filename[BUFFER_SIZE];
        char scanFormat[SCAN_FORMAT_SIZE];

        // scanf "%(BUFFER_SIZE)s %(BUFFER_SIZE)s %d"
        sprintf(scanFormat, SCAN_FORMAT, BUFFER_SIZE);
        if (scanf(scanFormat, filename, &fileQty) != 2) {
            errno = EINVAL;
            failNExit("Invalid stdin arguments");
        }

       fifoFD = open(filename, O_RDONLY);
    } else if (argc == 3) {
        // Get arguments from argv
        fileQty = strtol(argv[2], NULL, 10); // Text to NUM (Base 10)
        fifoFD = open(argv[1], O_RDONLY);

    } else {
        errno = EINVAL;
        failNExit("Invalid arguments");
    }

    if(fifoFD==-1){
        failNExit("error opening fifo pipe");
    }



    if (fileQty < 0) {
        failNExit("Error retrieving fileQty");
    }

    // Read shared memory once per file
    char buffer[BUFFER_SIZE] = {0};

    for (int i = 0; i < fileQty; i++) {
        int bytesRead = read(fifoFD, buffer, BUFFER_SIZE - 1);

        if (bytesRead == -1) {
            failNExit("Error reading from pipe");
        }

        buffer[bytesRead] = 0;

        printf("%s" , buffer);
    }

    if(close(fifoFD) ==-1){
        failNExit("error closing fifo pipe");
    }
    return 0;
}

static void failNExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
