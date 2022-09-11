// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "smADT.h"
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

#define READ 0
#define WRITE 1
#define SLAVE_PATH "./slave"
#define MAX(a,b) ((a)>(b)? (a):(b))

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
    fd_set rfds;
    int to_be_read;

} smCDT;

#define MAX_FILES_PER_SLAVE 4

static void sendFile(smADT sm,fd_t fd, char * filename);
static int startSlaves(smADT sm);
static void failNExit(const char *msg);

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
    FD_ZERO(&new->rfds);

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
    int nfds=-1;

    // if there are remaining fds to read, read them before calling select
    //si le agrego el if toberead<=0 se cuelga
    if (sm->to_be_read<=0){
        for (int i = 0; i < sm->sqty; i++){
            FD_SET(sm->pipe[i].out, &sm->rfds);
            nfds=MAX(nfds,sm->pipe[i].out);
        }

        sm->to_be_read = select(nfds+1, &sm->rfds, NULL, NULL, NULL);
        if(sm->to_be_read == -1) {
            freeSm(sm);
            return -1;
        }
    }

    int sindex = -1;
    int i = 0;
    while(sindex == -1 && i < sm->sqty) {
        if(FD_ISSET(sm->pipe[i].out, &sm->rfds)) {
            sindex = i;
        }
        i++;
    }

    if(sindex == -1) {
        return -1;
    }

    char c;
    while( bytesRead < bytes-2 && read(sm->pipe[sindex].out, &c, 1) > 0 && c != '\0' && c != '\n') {
        buff[bytesRead++] = c;
    }
    buff[bytesRead++] = '\n';
    buff[bytesRead] = '\0';

    sm->to_be_read--;
    FD_CLR(sm->pipe[sindex].out, &sm->rfds);
    sm->f_processed++;
    if(sm->f_sent != sm->filec) {
        sendFile(sm,sm->pipe[sindex].in, sm->filev[sm->f_sent]);
        sm->f_sent++;
    }
    return bytesRead;
}

void freeSm(smADT sm) {
    for (int i = 0; i < sm->sqty; i++){
        close(sm->pipe[i].in);
        close(sm->pipe[i].out);
    }

    free(sm->pipe);
    free(sm);
    return;
}

int hasNextFile(smADT sm){
    return sm->f_processed < sm->filec;
}

static void sendFile(smADT sm, fd_t fd, char * filename) {
    if (sm->f_sent < sm->filec)
    {
       dprintf(fd, "%s\n",filename);

    }
    return 1;

}


static int startSlaves(smADT sm) {
    if(sm == NULL) {
        errno = ENOMEM;
        return -1;
    }

    for(int i = 0; i < sm->sqty; i++) {
        fd_t ptc[2];
        fd_t ctp[2];

        if(-1 == pipe(ptc) || -1 == pipe(ctp)) {
            return -1;
        }

        sm->pipe[i].in=ptc[WRITE];
        sm->pipe[i].out=ctp[READ];


        int pid = fork();
        switch(pid) {
            case -1:
                return -1;
                break;
            case 0:

                // Close other slave processes pipes
                for (int j = 0; j < i; j++){
                    if(-1==close(sm->pipe[j].in))
                        failNExit("Error when closing pipe read end1");
                    if(-1==close(sm->pipe[j].out))
                        failNExit("Error when closing pipe write end2");
                }

                // Connect father to child pipe to child stdin
                if (close(ptc[WRITE]) == -1) {
                    failNExit("Error when closing pipe write end3 ");
                }

                // Connect child to father pipe to child stdout
                if (close(ctp[READ]) == -1) {
                    failNExit("Error when closing pipe read end5");
                }

                if (dup2(ptc[READ], STDIN_FILENO) == -1) {
                    failNExit("Error when duplicating pipe4");
                }

                if (dup2(ctp[WRITE], STDOUT_FILENO) == -1) {
                    failNExit("Error when duplicating pipe6");
                }

                execv(SLAVE_PATH, (char **)NULL);

                // If execv fails, it returns and we exit
                failNExit("Error with execv7");

                break;

            default:
                if(-1==close(ptc[READ])){
                    failNExit("Error when closing pipe read end8");
                }
                if(-1==close(ctp[WRITE])){
                    failNExit("Error when closing pipe write end9");
                }

                sendFile(sm, sm->pipe[i].in, sm->filev[sm->f_sent++]);
                sendFile(sm,sm->pipe[i].in, sm->filev[sm->f_sent++]);
                break;
        }
    }
    return 1;
}

static void failNExit(const char *msg) {
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}
