// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
#define _GNU_SOURCE
#include "md5.h"
#include "ADTs/pshmADT.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int checkPath(const char *path);
static int getFileData(int fd, char *buffer, int length);
static void setFileData(int fd, char *file);

typedef struct slave_info {
    pid_t pid;
    int pipe_father_to_child[2];
    int pipe_child_to_father[2];
    char *file_path;
    char md5[32]; // deberia ser 32 + 1 prob
} Tslave_info;

#define SLAVE_COUNT 1
#define READ 0
#define WRITE 1

int main(int argc, char *argv[]) {

    // Error in case of bad call
    if (argc == 1) {
        failNExit("Usage: md5sum <file1> <file2> ...");
    }

    int fileQty = 0;
    for (int i = 1; i < argc; i++) {
        if (checkPath(argv[i]))
            fileQty++;
    }

    // Initialize shared memory
    pshmADT pshm = newPshm("shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    // Print views arguments for use with pipe
    printf("%s %d", "shm", fileQty);

    // Create output file
    int outputFileFd = open("outputFile", O_CREAT | O_WRONLY | O_TRUNC, 00666);

    char *slave_path = "./slave";
    char *slave_args[] = {slave_path, NULL};
    Tslave_info slaves[SLAVE_COUNT] = {0};

    // Initialize slaves
    for (int i = 0; i < SLAVE_COUNT; i++) {
        // Create pipes for each slave
        if (pipe(slaves[i].pipe_father_to_child) == -1) {
            failNExit("Error when initializing father to child pipe");
        }

        if (pipe(slaves[i].pipe_child_to_father) == -1) {
            failNExit("Error when initializing child to father pipe");
        }

        // Fork
        slaves[i].pid = fork();
        if (slaves[i].pid == -1) {
            failNExit("Error when initializing slave");
        }

        // Connect pipes between father and child
        if (slaves[i].pid == 0) {

            // Connect father to child pipe to child stdin
            if (close(slaves[i].pipe_father_to_child[WRITE]) == -1) {
                failNExit("Error when closing pipe write end");
            }

            if (dup2(slaves[i].pipe_father_to_child[READ], STDIN_FILENO) == -1) {
                failNExit("Error when duplicating pipe");
            }

            // Connect child to father pipe to child stdout
            if (close(slaves[i].pipe_child_to_father[READ]) == -1) {
                failNExit("Error when closing pipe read end");
            }

            if (dup2(slaves[i].pipe_child_to_father[WRITE], STDOUT_FILENO) == -1) {
                failNExit("Error when duplicating pipe");
            }

            execv(slave_path, slave_args);

            // If execv fails, it returns and we exit
            failNExit("Error with execv");

        } else if (slaves[i].pid < 0) {

            failNExit("Error when using fork");
        }
        // Parent
        // Close unused pipe ends
        if (close(slaves[i].pipe_father_to_child[READ]) == -1) {
            failNExit("Error when closing pipe read end");
        }

        if (close(slaves[i].pipe_child_to_father[WRITE]) == -1) {
            failNExit("Error when closing pipe write end");
        }
    }

    // Sleep and send two files
    sleep(2);

    char *current_file = argv[1];

    if (fileQty >= 2) {
        dprintf(slaves[0].pipe_father_to_child[WRITE], "%s", current_file);
        dprintf(slaves[0].pipe_father_to_child[WRITE], "\n");
        current_file = argv[2];
        dprintf(slaves[0].pipe_father_to_child[WRITE], "%s", current_file);
        dprintf(slaves[0].pipe_father_to_child[WRITE], "\n");
    }

    char buffer[4000] = {0};

    int readFiles = 0;
    int writtenFiles = 2;

    // Read and Write files
    while (readFiles < fileQty) {
        int nByte = getFileData(slaves[0].pipe_child_to_father[READ], buffer, 4000);
        buffer[nByte] = 0;
        if (writtenFiles < fileQty) {
            current_file = argv[writtenFiles];
            setFileData(slaves[0].pipe_father_to_child[WRITE], current_file);
            writtenFiles++;
        }

        printf("%s", buffer);
        write(outputFileFd, buffer, nByte);
        writePshm(pshm, buffer, nByte);
        readFiles++;
    }

    return 0;
}

static void failNExit(const char *msg) {
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}

static int getFileData(int fd, char *buffer, int length) {
    return read(fd, buffer, length);
}

static void setFileData(int fd, char *file) {
    dprintf(fd, "%s", file);
    dprintf(fd, "\n");
}

static int checkPath(const char *path) {
    struct stat validation;
    return stat(path, &validation) >= 0 && S_ISREG(validation.st_mode);
}

// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// http://www.viva64.com
