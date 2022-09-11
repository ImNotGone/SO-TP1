// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "md5.h"

static int checkPath(const char *path);
static void failNExit(const char *msg);

#define READ 0
#define WRITE 1

int main(int argc, char *argv[]) {

    setvbuf(stdout, NULL, _IONBF, 0);

    // Error in case of bad call
    if (argc == 1) {
        failNExit("Usage: md5sum <file1> <file2> ...");
    }

    // Check files, TODO: only send correct files for processing or send all and check in child
    int fileQty = 0;
    for (int i = 1; i < argc; i++) {
        if (checkPath(argv[i]))
            fileQty++;
    }

    // Initialize shared memory
    pshmADT pshm = newPshm("shm", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    // Print views arguments for use with pipe
    printf("%s %d\n", "shm", fileQty);

    // Create output file
    int outputFileFd = open("outputFile", O_CREAT | O_WRONLY | O_TRUNC, 00666);


    sleep(2);

    char **files = &argv[1];

    // Initialize slaves
    smADT sm = newSm(fileQty, files, MIN_FILES_PER_SLAVE);

    // Retrieve info from slaves
    int bytesRead = 0;
    char buffer[BUFFER_SIZE] = {0};

    while (smHasFilesLeft(sm) > 0) {
        bytesRead = smRetrieve(sm, buffer, BUFFER_SIZE);
        if (bytesRead == -1) {
            failNExit("Error when reading/sending from/to slave");
        }

        // Print to output file & shared memory
        if (bytesRead > 0) {
            write(outputFileFd, buffer, bytesRead);
            writePshm(pshm, buffer, bytesRead);
        }
    }

    // Free resources
    close(outputFileFd);
    freeSm(sm);
    freePshm(pshm);
    return 0;
}

static void failNExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static int checkPath(const char *path) {
    struct stat validation;
    return stat(path, &validation) >= 0 && S_ISREG(validation.st_mode);
}
