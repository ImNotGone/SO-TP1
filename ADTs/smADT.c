// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _GNU_SOURCE
#include "smADT.h"

typedef struct slave_pipe {
    // Pipe fds
    int father_to_child;
    int child_to_father;
} slave_pipe_t;

typedef struct smCDT {
    // Slave info
    int slaves_dim;
    int slaves_qty;
    slave_pipe_t *slave_pipes;

    // Files to process
    int files_sent;
    int files_received;
    int file_qty;
    char **files;

    // Set for select
    fd_set child_to_father_fds;

} smCDT;

#define WRITE 1
#define READ 0
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void sendFile(smADT sm, char *file, int pipeFd);
static int readFile(smADT sm, char *buffer, int bufferSize, int pipeFd);
static int pollSlaves(smADT sm);
static int createSlave(smADT sm);
static int calculateMaxSlaveQty(int fileQty, int minFilesPerSlave);

smADT newSm(int fileQty, char **files, int minFilesPerSlave) {
    smADT new = malloc(sizeof(smCDT));
    if (new == NULL) {
        return NULL;
    }

    // Slaves
    new->slaves_dim = calculateMaxSlaveQty(fileQty, minFilesPerSlave); // +1 = celing criollo

    new->slave_pipes = malloc(sizeof(slave_pipe_t) * new->slaves_dim);
    if (new->slave_pipes == NULL) {
        free(new);
        return NULL;
    }

    new->slaves_qty = 0;

    // Files
    new->file_qty = fileQty;
    new->files_sent = 0;
    new->files_received = 0;
    new->files = files;

    // Initialize file descriptor set
    FD_ZERO(&new->child_to_father_fds);

    return new;
}

int smHasFilesLeft(smADT sm) {
    if (sm == NULL) {
        return -1;
    }

    return sm->files_received < sm->file_qty;
}

ssize_t smRetrieve(smADT sm, char *buffer, int bufferSize) {

    // Update file descriptor set
    if (pollSlaves(sm) == -1) {
        return -1;
    }

    // Send file to first available slave and read response
    for (int i = 0; i < sm->slaves_qty; i++) {
        slave_pipe_t *currentSlave = &sm->slave_pipes[i];

        // Check if slave is available
        if (FD_ISSET(currentSlave->child_to_father, &sm->child_to_father_fds)) {

            // Retrieve info from slave
            int bytesRead = readFile(sm, buffer, bufferSize, currentSlave->child_to_father);

            // Send next file to slave
            sendFile(sm, sm->files[sm->files_sent], currentSlave->father_to_child);
            return bytesRead;
        }
    }

    // No slave available, create a new one if needed
    if (sm->slaves_qty < sm->slaves_dim) {
        createSlave(sm);
    }

    return 0;
}

void freeSm(smADT sm) {

    // Free slave resources
    for (int i = 0; i < sm->slaves_qty; i++) {
        close(sm->slave_pipes[i].father_to_child);
        close(sm->slave_pipes[i].child_to_father);
    }

    // Free sm resources
    free(sm->slave_pipes);
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
    if (sm->files_sent >= sm->file_qty) {
        return;
    }

    dprintf(pipeFd, "%s\n", file);
    sm->files_sent++;
}

// Reads info from slave
static int readFile(smADT sm, char *buffer, int bufferSize, int pipeFd) {
    int bytesRead = read(pipeFd, buffer, bufferSize);
    buffer[bytesRead] = '\0';

    sm->files_received++;

    return bytesRead + 1;
}

// TODO: better error handling
static int createSlave(smADT sm) {
    if (sm == NULL) {
        errno = ENOMEM;
        return -1;
    }

    slave_pipe_t *newSlave = malloc(sizeof(slave_pipe_t));

    // Create pipes
    int parentToChild[2];
    int childToParent[2];

    if (pipe(parentToChild) == -1 || pipe(childToParent) == -1) {
        return -1;
    }

    // Assign pipes
    newSlave->father_to_child = parentToChild[WRITE];
    newSlave->child_to_father = childToParent[READ];

    // Update slave info
    sm->slave_pipes[sm->slaves_qty] = *newSlave;
    sm->slaves_qty++;

    // Send first file
    sendFile(sm, sm->files[sm->files_sent], newSlave->father_to_child);

    // Fork
    int pid = fork();
    if (pid < 0) {
        return -1;
    }
    // Connect pipes between father and child
    if (pid == 0) {

        // Close other slave slave pipes
        for (int i = 0; i < sm->slaves_qty - 1; i++) {
            close(sm->slave_pipes[i].father_to_child);
            close(sm->slave_pipes[i].child_to_father);
        }

        // Connect father to child pipe to child stdin
        if (close(parentToChild[WRITE]) == -1) {
            exit(EXIT_FAILURE);
        }

        if (dup2(parentToChild[READ], STDIN_FILENO) == -1) {
            exit(EXIT_FAILURE);
        }

        // Connect child to father pipe to child stdout
        if (close(childToParent[READ]) == -1) {
            exit(EXIT_FAILURE);
        }

        if (dup2(childToParent[WRITE], STDOUT_FILENO) == -1) {
            exit(EXIT_FAILURE);
        }

        // Execute slave progran
        execv(SLAVE_PATH, (char **){NULL});

        exit(EXIT_FAILURE);
    }

    // Parent
    // Close unused pipe ends
    if (close(parentToChild[READ]) == -1) {
        return -1;
    }

    if (close(childToParent[WRITE]) == -1) {
        return -1;
    }

    return 1;
}

static int pollSlaves(smADT sm) {

    int nfds = -1;

    struct timeval timeout = {.tv_sec = 0, .tv_usec = 0};

    for (int i = 0; i < sm->slaves_qty; i++) {
        FD_SET(sm->slave_pipes[i].child_to_father, &sm->child_to_father_fds);
        nfds = MAX(nfds, sm->slave_pipes[i].child_to_father);
    }

    // Check which slaves have finished
    if (select(nfds + 1, &sm->child_to_father_fds, NULL, NULL, sm->slaves_qty == sm->slaves_dim ? NULL : &timeout) == -1) {
        freeSm(sm);
        return -1;
    }


    return 0;
}
