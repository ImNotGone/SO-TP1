// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _GNU_SOURCE
#include "smADT.h"

typedef struct slave_pipe {
    // Pipe fds
    int fatherToChild;
    int childToFather;
} slave_pipe_t;

typedef struct smCDT {
    // Slave info
    int slavesDim;
    int slavesQty;
    slave_pipe_t *slavePipes;

    // Files to process
    int filesSent;
    int filesReceived;
    int fileQty;
    char **files;

    // Set for select
    fd_set childToFatherFds;

} smCDT;

#define READ 0
#define WRITE 1
#define ERROR -1
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void sendFile(smADT sm, char *file, int pipeFd);
static int readFile(smADT sm, char *buffer, int bufferSize, int pipeFd);
static int pollSlaves(smADT sm);
static int createSlave(smADT sm);
static int calculateMaxSlaveQty(int fileQty, int minFilesPerSlave);

smADT newSm(int fileQty, char **files, int minFilesPerSlave) {
    if(files == NULL || minFilesPerSlave <= 0 || fileQty <= 0) {
        errno = EINVAL;
        return NULL;
    }

    smADT new = malloc(sizeof(smCDT));
    if (new == NULL) {
        return NULL;
    }

    // Slaves
    new->slavesDim = calculateMaxSlaveQty(fileQty, minFilesPerSlave); // +1 = celing criollo

    new->slavePipes = malloc(sizeof(slave_pipe_t) * new->slavesDim);
    if (new->slavePipes == NULL) {
        freeSm(new);
        return NULL;
    }

    new->slavesQty = 0;

    // Files
    new->fileQty = fileQty;
    new->filesSent = 0;
    new->filesReceived = 0;
    new->files = files;

    // Initialize file descriptor set
    FD_ZERO(&new->childToFatherFds);

    return new;
}

int smHasFilesLeft(smADT sm) {
    if (sm == NULL) {
        errno = EINVAL;
        return 0;
    }

    return sm->filesReceived < sm->fileQty;
}

ssize_t smRetrieve(smADT sm, char *buffer, int bufferSize) {

    // Update file descriptor set
    if (pollSlaves(sm) == ERROR) {
        return ERROR;
    }

    // Send file to first available slave and read response
    for (int i = 0; i < sm->slavesQty; i++) {
        slave_pipe_t *currentSlave = &sm->slavePipes[i];

        // Check if slave is available
        if (FD_ISSET(currentSlave->childToFather, &sm->childToFatherFds)) {

            // Retrieve info from slave
            int bytesRead = readFile(sm, buffer, bufferSize, currentSlave->childToFather);

            if (bytesRead == ERROR) {
                return ERROR;
            }

            // Send next file to slave
            sendFile(sm, sm->files[sm->filesSent], currentSlave->fatherToChild);
            return bytesRead;
        }
    }

    // No slave available, create a new one if needed
    if (sm->slavesQty < sm->slavesDim) {
        if (createSlave(sm) == ERROR) {
            return ERROR;
        }
    }

    return 0;
}

void freeSm(smADT sm) {
    if (sm == NULL) {
        return;
    }

    if (sm->slavePipes != NULL) {
        // Free slave resources
        for (int i = 0; i < sm->slavesQty; i++) {
            close(sm->slavePipes[i].fatherToChild);
            close(sm->slavePipes[i].childToFather);
        }
        free(sm->slavePipes);
    }

    free(sm);
    return;
}

// Up to minFilesPerSlave files per slave, if there are more files than minFilesPerSlave
static int calculateMaxSlaveQty(int fileQty, int minFilesPerSlave) {
    if (fileQty < minFilesPerSlave) {
        return fileQty;
    }

    int remainder = fileQty % minFilesPerSlave;
    return (fileQty / minFilesPerSlave) + (remainder == 0 ? 0 : 1);
}

// Sends info to slave
static void sendFile(smADT sm, char *file, int pipeFd) {
    // Do not send if there are no more files
    if (sm->filesSent >= sm->fileQty) {
        return;
    }

    dprintf(pipeFd, "%s\n", file);
    sm->filesSent++;
}

// Reads info from slave
static int readFile(smADT sm, char *buffer, int bufferSize, int pipeFd) {
    int bytesRead = read(pipeFd, buffer, bufferSize);

    if (bytesRead == ERROR) {
        return ERROR;
    }

    buffer[bytesRead] = '\0';

    sm->filesReceived++;

    return bytesRead + 1;
}

// TODO: better error handling
static int createSlave(smADT sm) {
    if (sm == NULL) {
        errno = EINVAL;
        return ERROR;
    }

    slave_pipe_t *newSlave = malloc(sizeof(slave_pipe_t));
    
    if (newSlave == NULL) {
        return ERROR;
    }

    // Create pipes
    int parentToChild[2];
    int childToParent[2];

    if (pipe(parentToChild) == ERROR || pipe(childToParent) == ERROR) {
        return ERROR;
    }

    // Assign pipes
    newSlave->fatherToChild = parentToChild[WRITE];
    newSlave->childToFather = childToParent[READ];

    // Update slave info
    sm->slavePipes[sm->slavesQty] = *newSlave;
    sm->slavesQty++;

    // Send first file
    sendFile(sm, sm->files[sm->filesSent], newSlave->fatherToChild);

    // Fork
    int pid = fork();
    if (pid < 0) {
        return ERROR;
    }
    // Connect pipes between father and child
    if (pid == 0) {

        // Close other slave & unused parent pipe ends
        for (int i = 0; i < sm->slavesQty - 1; i++) {
            if (close(sm->slavePipes[i].fatherToChild) == ERROR || close(sm->slavePipes[i].childToFather) == ERROR) {
                exit(EXIT_FAILURE);;
            }
        }

        if (close(parentToChild[WRITE]) == ERROR || close(childToParent[READ]) == ERROR) {
            exit(EXIT_FAILURE);;
        }

        // Connect father to child pipe to child stdin & child to father pipe to child stdout
        if (dup2(parentToChild[READ], STDIN_FILENO) == ERROR || dup2(childToParent[WRITE], STDOUT_FILENO) == ERROR) {
            exit(EXIT_FAILURE);
        }

        // Execute slave program, if it fails it returns and we exit
        execv(SLAVE_PATH, (char **){NULL});

        exit(EXIT_FAILURE);
    }

    // Parent
    // Close unused slave pipe ends
    if (close(parentToChild[READ]) == ERROR || close(childToParent[WRITE]) == ERROR) {
        return ERROR;
    }

    return 0;
}

static int pollSlaves(smADT sm) {
    if(sm == NULL) {
        errno = EINVAL;
        return ERROR;
    }
    
    int nfds = -1;

    struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};

    for (int i = 0; i < sm->slavesQty; i++) {
        FD_SET(sm->slavePipes[i].childToFather, &sm->childToFatherFds);
        nfds = MAX(nfds, sm->slavePipes[i].childToFather);
    }

    // Check which slaves have finished
    if (select(nfds + 1, &sm->childToFatherFds, NULL, NULL, sm->slavesQty == sm->slavesDim ? NULL : &timeout) == ERROR) {
        freeSm(sm);
        return ERROR;
    }
    
    return 0;
}
