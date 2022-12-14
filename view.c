// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "view.h"
#include "ADTs/pshmADT.h"

#define SCAN_FORMAT "%%%ds %%d"
#define SCAN_FORMAT_SIZE 16

static void failNExit(const char *msg);

int main(int argc, char *argv[]) {

    pshmADT pshm = NULL;
    int fileQty = 0;

    // Parse arguments
    if (argc == 1) {
        // Read arguments from stdin ($> pshmName fileQty)
        char pshmName[BUFFER_SIZE];
        char scanFormat[SCAN_FORMAT_SIZE];

        // scanf "%(BUFFER_SIZE)s %(BUFFER_SIZE)s %d"
        sprintf(scanFormat, SCAN_FORMAT, BUFFER_SIZE);
        if (scanf(scanFormat, pshmName, &fileQty) != 2) {
            errno = EINVAL;
            failNExit("Invalid stdin arguments");
        }

        pshm = newPshm(pshmName, O_RDWR, S_IRUSR | S_IWUSR);

    } else if (argc == 3) {
        // Get arguments from argv
        pshm = newPshm(argv[1], O_RDWR, S_IRUSR | S_IWUSR);
        fileQty = strtol(argv[2], NULL, 10); // Text to NUM (Base 10)

    } else {
        errno = EINVAL;
        failNExit("Invalid arguments");
    }

    // Validate shared memory is vaild
    if (pshm == NULL) {
        failNExit("Error creating pshm");
    }

    if (fileQty < 0) {
        freePshm(pshm);
        failNExit("Error retrieving fileQty");
    }

    // Read shared memory once per file
    char buffer[BUFFER_SIZE] = {0};

    for (int i = 0; i < fileQty; i++) {
        int bytesRead = readPshm(pshm, buffer, BUFFER_SIZE);
        if (bytesRead == -1) {
            freePshm(pshm);
            failNExit("Error reading pshm");
        }

        printf("%s", buffer);
    }
    putchar('\n');

    freePshm(pshm);
    return 0;
}

static void failNExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
