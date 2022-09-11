// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _GNU_SOURCE
#include "md5.h"
#include "ADTs/pshmADT.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "ADTs/smADT.h"
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

#define MAX_OUTPUT 4096
#define READ 0
#define WRITE 1

int main(int argc, char *argv[]) {

    setvbuf(stdout, NULL, _IONBF, 0);
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


    // Create output file
    int outputFileFd = open("outputFile", O_CREAT | O_WRONLY | O_TRUNC, 00666);

    char *slave_path = "./slave";
    char *slave_args[] = {slave_path, NULL};

    char ** files = &argv[1];

    // Initialize slaves
    smADT sm = newSm(fileQty, files);

    sleep(2);

    printf("%s %d\n", "shm", fileQty);

    char buffer[MAX_OUTPUT] = {0};

    while(hasNextFile(sm)){
        int nBytes= smRead(sm, buffer, MAX_OUTPUT);
        if(write(outputFileFd, buffer,nBytes)== -1){
            failNExit("Writing error");
        }
        writePshm(pshm, buffer, nBytes);
    }

    freePshm(pshm);
    freeSm(sm);
    close(outputFileFd);
    return 0;
}

static void failNExit(const char *msg) {
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}

static int checkPath(const char *path) {
    struct stat validation;
    return stat(path, &validation) >= 0 && S_ISREG(validation.st_mode);
}
