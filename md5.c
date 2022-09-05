#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct slave_info {
    pid_t pid;
    int pipe_fd[2];
    char * file_path;
    char md5[32]; // deberia ser 32 + 1 prob
}Tslave_info;

#define SLAVE_COUNT 5

int main(int argc, char * argv[]) {

    // Error in case of bad call
    if(argc == 1) {
        fprintf(stderr, "Usage: %s <files>", argv[0]);
        exit(EXIT_FAILURE);
    }

    char * slave_path = "./slave";
    char * slave_args[] = {slave_path, NULL};
    Tslave_info slaves[SLAVE_COUNT] = {0};
    // initialize pipes
    for(int i = 0; i < SLAVE_COUNT; i++) {
        // create a pipe for each slave
        if(pipe(slaves[i].pipe_fd) == -1) {
            fprintf(stderr, "Error when initializing pipe %d", i);
            exit(EXIT_FAILURE);
        }
        // fork
        slaves[i].pid = fork();
        if(slaves[i].pid == -1) {
            fprintf(stderr, "Error when initializing slave %d", i);
            exit(EXIT_FAILURE);
        }

        // create pipe for slave process
        if(slaves[i].pid == 0) {
            dup2(slaves[i].pipe_fd[0], STDIN_FILENO);
            dup2(slaves[i].pipe_fd[1], STDOUT_FILENO);
            close(slaves[i].pipe_fd[0]);
            close(slaves[i].pipe_fd[1]);
            execv(slave_path, slave_args);
            // not necessary (execv takes over process)
            // return;
        } else { // choose what to do on parent

        }
    }

    // pass files to slaves
    char * current_file;
    for(int i = 1; i < argc; i++) {
        current_file = argv[i];
        // handle file in slave process
    }
    return 0;
}
