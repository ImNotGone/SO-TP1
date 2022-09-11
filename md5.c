// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "md5.h"

#define READ 0
#define WRITE 1
#define MD5_LOG_FILE_NAME "md5.log"

typedef int fd_t;

static void failNExit(const char *msg);

int main(int argc, char *argv[]) {

    setvbuf(stdout, NULL, _IONBF, 0);

    // Error in case of bad call
    if (argc == 1) {
        char errorBuff[MAX_OUTPUT];
        snprintf(errorBuff, MAX_OUTPUT, "Usage: %s <file1> <file2>", argv[0]);
        failNExit(errorBuff);
    }

    // Initialize shared memory
    pshmADT pshm = newPshm("shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (pshm == NULL) {
        failNExit("Error creating shared memory");
    }

    // Create output file
    fd_t md5LogFile = open(MD5_LOG_FILE_NAME, O_CREAT | O_WRONLY | O_TRUNC, 00666);
    if (md5LogFile == -1) {
        failNExit("Error opening log file");
    }

    // Files to process
    char ** files = &argv[1];
    int fileQty = argc - 1;

    // Communicate with view process
    printf("%s %d\n", "shm", fileQty);

    // Initialize slaves
    smADT sm = newSm(fileQty, files, PATH_TO_SLAVE);
    if (sm == NULL) {
        failNExit("Error creating slave manager");
    }

    sleep(2);

    char buffer[MAX_OUTPUT] = {0};

    while(hasNextFile(sm)){
        // Read from slave
        int nBytes= smRead(sm, buffer, MAX_OUTPUT);
        if (nBytes == -1) {
            failNExit("Error reading from slave");
        }

        // Print to output file and shared memory
        if (write(md5LogFile, buffer,nBytes) == -1) {
            failNExit("Error writing to log file");
        }

        if(writePshm(pshm, buffer, nBytes) == 0){
            failNExit("Error writing to shared memory");
        }
    }

    // Free resources
    freePshm(pshm);
    freeSm(sm);

    if (close(md5LogFile) == -1) {
        failNExit("Error closing log file");
    }

    return 0;
}

static void failNExit(const char *msg) {
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}
