// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
    switch(argc) {
        case 1:
            // TODO: handle reading shared memory connection info
            break;
        case 2:
            // TODO: handle recieving shared memory connection info via argument
            break;
        default:
            fprintf(stderr, "Wrong view usage\n");
            exit(EXIT_FAILURE);
    }
    return 0;
}

// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
