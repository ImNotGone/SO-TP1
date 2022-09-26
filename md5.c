// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "md5.h"
#include <sys/types.h>
#include <sys/stat.h>

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
    if(mkfifo("grupo15", 0777) == -1){
        if(errno != EEXIST){
            failNExit("Error making fifo pipe");
        }
    }


    // Files to process
    char ** files = &argv[1];
    int fileQty = argc - 1;

    // Communicate with view process
    printf("%s %d\n", "grupo15", fileQty);

    // Initialize slaves
    smADT sm = newSm(fileQty, files);
    if (sm == NULL) {
        failNExit("Error creating slave manager");
    }

    int entry = open("grupo15", O_RDWR);
    if(entry == -1){
        failNExit("Error opening fifo pipe");
    }

    sleep(2);

    char buffer[MAX_OUTPUT] = {0};

    // Create output file
    fd_t md5LogFile = open(MD5_LOG_FILE_NAME, O_CREAT | O_WRONLY | O_TRUNC, 00666);
    if (md5LogFile == -1) {
        freeSm(sm);
        failNExit("Error opening log file");
    }



    while(hasNextFile(sm)){
        // Read from slave
        int nBytes= smRead(sm, buffer, MAX_OUTPUT);
        if (nBytes == -1) {
            freeSm(sm);
            failNExit("Error reading from slave");
        }

        // Print to output file and shared memory
        if (write(md5LogFile, buffer,nBytes) == -1) {
            freeSm(sm);
            failNExit("Error writing to log file");
        }

        if(write(entry, buffer, nBytes)== -1){
            failNExit("error writing to fifo pipe");
        }
    }

    // Free resources
    freeSm(sm);

    if (close(md5LogFile) == -1) {
        failNExit("Error closing log file");
    }

    if(close(entry)== -1){
        failNExit("Error closing fifo pipe");
    }

    remove("grupo15");

    return 0;
}

static void failNExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
