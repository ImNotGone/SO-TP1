// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "smADT.h"

typedef struct slave_info {
    // Pipe fds
    int father_to_child;
    int child_to_father;

    // Files to process
    char **files;
    int file_qty;
    int files_sent;
    int files_received;
} slave_info_t;

typedef struct smCDT {
    // Slaves to process files
    int slave_qty;
    slave_info_t *slaves;

    // Set for select
    fd_set child_to_father_fds;
} smCDT;

#define WRITE 1
#define READ 0
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void sendFile(slave_info_t *slave);
static int readFile(slave_info_t *slave, char *buffer, int bufferSize);
static int pollSlaves(smADT sm);
static int startSlaves(smADT sm, char **files, int fileQty);
static int calculateSlaveQty(int fileQty);

smADT newSm(int fileQty, char **files) {
    smADT new = malloc(sizeof(smCDT));
    if (new == NULL) {
        return NULL;
    }

    // Slaves
    new->slave_qty = calculateSlaveQty(fileQty); // +1 = celing criollo

    new->slaves = malloc(sizeof(slave_info_t) * new->slave_qty);
    if (new->slaves == NULL) {
        free(new);
        return NULL;
    }

    // Initialize file descriptor set
    FD_ZERO(&new->child_to_father_fds);

    if (startSlaves(new, files, fileQty) == -1) {
        freeSm(new);
        return NULL;
    }
    
    return new;
}

int smFilesLeft(smADT sm) {
    if (sm == NULL) {
        return -1;
    }

    // Check if there are files left to process in any slave
    slave_info_t currentSlave;
    for (int i = 0; i < sm->slave_qty; i++) {
        currentSlave = sm->slaves[i];
        if (currentSlave.files_sent < currentSlave.file_qty || currentSlave.files_received < currentSlave.file_qty) {
            return 1;
        }
    }

    return 0;
}

ssize_t smSendNRead(smADT sm, char *buffer, int bufferSize) {

    // Update file descriptor set
    if (pollSlaves(sm) == -1) {
        return -1;
    }

    // Send file to first available slave and read response
    for (int i = 0; i < sm->slave_qty; i++) {
        slave_info_t *currentSlave = &sm->slaves[i];

        // Check if slave is available
        if (FD_ISSET(currentSlave->child_to_father, &sm->child_to_father_fds)) {

            // Retrieve info from slave
            int bytesRead = readFile(currentSlave, buffer, bufferSize);

            // Send next file to slave
            sendFile(currentSlave);
            return bytesRead;
        }
    }

    return -1;
}

void freeSm(smADT sm) {

    // Free slave resources
    for (int i = 0; i < sm->slave_qty; i++) {
        close(sm->slaves[i].father_to_child);
        close(sm->slaves[i].child_to_father);
        free(sm->slaves[i].files);
    }

    // Free sm resources
    free(sm->slaves);
    free(sm);
    return;
}

// Up to FILES_PER_SLAVE files per slave, if there are more files than FILES_PER_SLAVE
static int calculateSlaveQty(int fileQty) {
    if (fileQty < FILES_PER_SLAVE) {
        return fileQty;
    }

    int remainder = fileQty % FILES_PER_SLAVE;
    return (fileQty / FILES_PER_SLAVE) + (remainder == 0 ? 0 : 1);
}

// Sends info to slave
static void sendFile(slave_info_t *slave) {
    // Do not read if all files have been sent
    if (slave->files_sent >= slave->file_qty || slave->files[slave->files_sent] == NULL) {
        return;
    }

    dprintf(slave->father_to_child, "%s\n", slave->files[slave->files_sent]);
    slave->files_sent++;
}

// Reads info from slave
static int readFile(slave_info_t *slave, char *buffer, int bufferSize) {
    int bytesRead = read(slave->child_to_father, buffer, bufferSize);
    buffer[bytesRead] = '\0';

    slave->files_received++;

    return bytesRead + 1;
}

// TODO: better error handling
static int startSlaves(smADT sm, char **files, int fileQty) {
    if (sm == NULL) {
        errno = ENOMEM;
        return -1;
    }

    // Start slaves
    slave_info_t *currentSlave;
    for (int i = 0; i < sm->slave_qty; i++) {
        currentSlave = &sm->slaves[i];

        // Assign files to each slave
        currentSlave->files = malloc(sizeof(char *) * FILES_PER_SLAVE);

        int j;
        for (j = 0; j < FILES_PER_SLAVE && i * FILES_PER_SLAVE + j < fileQty; j++) {
            currentSlave->files[j] = files[i * FILES_PER_SLAVE + j];
        }

        currentSlave->file_qty = j;
        currentSlave->files_sent = 0;
        currentSlave->files_received = 0;

        // Create pipes for each slave
        int parentToChild[2];
        int childToParent[2];

        if (pipe(parentToChild) == -1 || pipe(childToParent) == -1) {
            return -1;
        }

        sm->slaves[i].father_to_child = parentToChild[WRITE];
        sm->slaves[i].child_to_father = childToParent[READ];

        // Send first file to each slave
        sendFile(currentSlave);

        // Fork
        int pid = fork();
        if (pid < 0) {
            return -1;
        }
        // Connect pipes between father and child
        if (pid == 0) {

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

            // Execute slave & send first file
            execv(SLAVE_PATH, (char **) {NULL});

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
    }

    return 1;
}

static int pollSlaves(smADT sm) {
        int nfds = -1;

        for (int i = 0; i < sm->slave_qty; i++) {
            if (sm->slaves[i].files_received < sm->slaves[i].file_qty) {
                FD_SET(sm->slaves[i].child_to_father, &sm->child_to_father_fds);
                nfds = MAX(nfds, sm->slaves[i].child_to_father);
            }
        }

        // Check which slaves have finished
        if (select(nfds + 1, &sm->child_to_father_fds, NULL, NULL, NULL) == -1) {
            freeSm(sm);
            return -1;
        }

        return 0;
}
