#include "smADT.h"
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

typedef int fd_t;

typedef struct pipe {
    fd_t in;
    fd_t out;
} pipe_t;

typedef struct smCDT {
    int filec;
    int f_sent;
    int f_processed;
    char ** filev;


    int sqty;
    pipe_t * pipe;
    fd_set out_fds;
    int to_be_read;

} smCDT;

#define MAX_FILES_PER_SLAVE 4

static void sendFile(fd_t fd, char * filename);
static int startSlaves(smADT sm);

smADT newSm(int filec, char ** filev) {
    smADT new = malloc(sizeof(smCDT));
    if(new == NULL) {
        return NULL;
    }

    new->filec = filec;
    new->filev = filev;
    new->sqty = filec/MAX_FILES_PER_SLAVE + 1; // +1 = celing criollo
    new->to_be_read = 0;
    new->f_processed = 0;
    new->f_sent = 0;
    // initialize file descriptor set
    FD_ZERO(&new->out_fds);

    new->pipe = malloc(sizeof(pipe_t)*new->sqty);
    if(new->pipe == NULL) {
        free(new);
        return NULL;
    }

    if(startSlaves(new) == -1) {
        freeSm(new);
        return NULL;
    }
    return new;
}

ssize_t smRead(smADT sm, char * buff, size_t bytes) {
    ssize_t bytesRead = 0;


    // if there are remaining fds to read, read them before calling select
    if(sm->to_be_read == 0) {

        sm->to_be_read = select(sm->sqty, &sm->out_fds, NULL, NULL, NULL);
        if(sm->to_be_read == -1) {
            freeSm(sm);
            return -1;
        }
    }

    int sindex = -1;
    int i = 0;
    while(sindex == -1 && i < sm->sqty) {
        if(FD_ISSET(sm->pipe[i].out, &sm->out_fds)) {
            sindex = i;
        }
        i++;
    }

    if(sindex == -1) {
        return -1;
    }

    char c;
    while(  bytesRead < bytes && read(sm->pipe[sindex].out, &c, 1) > 0 \
            && c != '\0' && c != '\n') {
        buff[bytesRead++] = c;
    }
    buff[bytesRead++] = '\n';
    buff[bytesRead] = '\0';

    sm->to_be_read--;
    if(sm->f_processed++ != sm->filec) {
        sendFile(sm->pipe[sindex].in, sm->filev[sm->f_sent++]);
    }
    return bytesRead;
}

void freeSm(smADT sm) {
    free(sm->pipe);
    free(sm);
    return;
}

static void sendFile(fd_t fd, char * filename) {
    write(fd, filename, strlen(filename));
    write(fd, "\n", 1);
}


static int startSlaves(smADT sm) {
    if(sm == NULL) {
        errno = ENOMEM;
        return -1;
    }

    for(int i = 0; i < sm->filec; i++) {
        fd_t ptc[2];
        fd_t ctp[2];

        if(-1 == pipe(ptc) || -1 == pipe(ctp)) {
            return -1;
        }

        int pid = fork();
        switch(pid) {
            case -1: return -1; break;
            case 0:

                //TODO
                // dup y close file desc


                break;

            default:
                // TODO
                // close file desc
                sendFile(sm->pipe[i].in, sm->filev[sm->f_sent++]);
                sendFile(sm->pipe[i].in, sm->filev[sm->f_sent++]);
                break;
        }
    }
    return 1;
}
