// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "smADT.h"

#define READ 0
#define WRITE 1
#define MAX(a,b) ((a)>(b)? (a):(b))

typedef int fd_t;

typedef struct pipe {
    fd_t in;
    fd_t out;
} pipe_t;

typedef struct smCDT {
    // Files to process
    int fileQty;
    int filesSent;
    int filesProcessed;
    char ** files;

    // Slave info
    int slaveQty;
    pipe_t * pipes;
    fd_set readFds;
    int toBeRead;

} smCDT;

#define ERROR -1

static void sendFile(smADT sm, fd_t fd, char * filename);
static int startSlaves(smADT sm);
static int calculateSlaveQty(int fileQty, int filesPerSlave);

smADT newSm(int fileQty, char ** files) {
    if(files == NULL || fileQty <= 0) {
        errno = EINVAL;
        return NULL;
    }

    smADT new = malloc(sizeof(smCDT));
    if(new == NULL) {
        return NULL;
    }

    // Files
    new->fileQty = fileQty;
    new->files = files;
    new->filesProcessed = 0;
    new->filesSent = 0;

    // Slaves
    new->slaveQty = calculateSlaveQty(fileQty, MAX_FILES_PER_SLAVE);
    new->toBeRead = 0;
    new->pipes = malloc(sizeof(pipe_t)*new->slaveQty);
    if(new->pipes == NULL) {
        freeSm(new);
        return NULL;
    }

    // Initialize file descriptor set
    FD_ZERO(&new->readFds);

    // Start slaves
    if(startSlaves(new) == ERROR) {
        freeSm(new);
        return NULL;
    }
    return new;
}

ssize_t smRead(smADT sm, char * buff, size_t bytes) {
    ssize_t bytesRead = 0;

    // If there are remaining fds to read, read them before calling select
    if (sm->toBeRead <= 0) {
        int nfds = -1;

        // Add slave fd to set
        for (int i = 0; i < sm->slaveQty; i++) {
            FD_SET(sm->pipes[i].out, &sm->readFds);
            nfds = MAX(nfds,sm->pipes[i].out);
        }

        // Check which slaves have finished
        sm->toBeRead = select(nfds+1, &sm->readFds, NULL, NULL, NULL);
        if(sm->toBeRead == -1) {
            freeSm(sm);
            return ERROR;
        }
    }

    // Find first available slave
    int sindex = -1;
    int i = 0;
    while(sindex == -1 && i < sm->slaveQty) {
        if(FD_ISSET(sm->pipes[i].out, &sm->readFds)) {
            sindex = i;
        }
        i++;
    }

    // If no available slaves & select did not block
    if(sindex == -1) {
        errno = ECHILD;
        return ERROR;
    }

    // Read from slave
    char c;
    while(bytesRead < bytes - 2 && read(sm->pipes[sindex].out, &c, 1) > 0 && c != '\0' && c != '\n') {
        buff[bytesRead++] = c;
    }
    buff[bytesRead++] = '\n';
    buff[bytesRead] = '\0';
    sm->filesProcessed++;

    // Remove slave from available slave list
    sm->toBeRead--;
    FD_CLR(sm->pipes[sindex].out, &sm->readFds);

    // Send new file
    if (sm->filesSent != sm->fileQty) {
        sendFile(sm, sm->pipes[sindex].in, sm->files[sm->filesSent]);
        sm->filesSent++;
    }

    return bytesRead;
}

void freeSm(smADT sm) {
    if (sm == NULL) {
        errno = EINVAL;
        return;
    }

    // Free slave resources
    if (sm->pipes != NULL) {
        for (int i = 0; i < sm->slaveQty; i++){
            if (close(sm->pipes[i].in) == ERROR || close(sm->pipes[i].out) == ERROR) {
                return;
            }
        }
        free(sm->pipes);
    }

    free(sm);
    return;
}

int hasNextFile(smADT sm){
    if (sm == NULL) {
        return 0;
    }

    return sm->filesProcessed < sm->fileQty;
}

static void sendFile(smADT sm, fd_t fd, char * filename) {
    if (sm->filesSent == sm->fileQty) {
        return;
    }

    dprintf(fd, "%s\n",filename);
}


static int startSlaves(smADT sm) {
    if(sm == NULL) {
        errno = EINVAL;
        return ERROR;
    }

    for(int i = 0; i < sm->slaveQty; i++) {
        fd_t ptc[2];
        fd_t ctp[2];

        // Create pipes
        if(pipe(ptc) == ERROR || pipe(ctp) == ERROR) {
            return ERROR;
        }

        sm->pipes[i].in = ptc[WRITE];
        sm->pipes[i].out = ctp[READ];


        int pid = fork();
        switch(pid) {
            case ERROR:
                return ERROR;
                break;
            case 0:

                // Close other slave & unused parent pipe ends
                for (int j = 0; j < i; j++){
                    if(close(sm->pipes[j].in) == ERROR || close(sm->pipes[j].out) == ERROR)
                        return ERROR;
                }

                if (close(ptc[WRITE]) == ERROR || close(ctp[READ]) == ERROR) {
                    return ERROR;
                }

                // Connect child to father pipe to child stdout & father to child pipe to stdin
                if (dup2(ptc[READ], STDIN_FILENO) == ERROR || dup2(ctp[WRITE], STDOUT_FILENO) == ERROR) {
                    return ERROR;
                }

                static char * const SLAVE_ARGS[] = {SLAVE_PATH, NULL};
                execv(SLAVE_PATH, SLAVE_ARGS);

                // If execv fails, it returns and we exit
                errno = ECHILD;
                exit(EXIT_FAILURE);

                break;

            default:
                // Close unused slave pipe ends
                if(close(ptc[READ]) == ERROR || close(ctp[WRITE]) == ERROR){
                    return ERROR;
                }

                // Send files
                sendFile(sm, sm->pipes[i].in, sm->files[sm->filesSent++]);
                sendFile(sm, sm->pipes[i].in, sm->files[sm->filesSent++]);

                break;
        }
    }
    return 1;
}

static int calculateSlaveQty(int fileQty, int filesPerSlave) {
    int remainder = fileQty % filesPerSlave;
    return (fileQty/filesPerSlave) + (remainder == 0 ? 0 : 1);
}
