#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static void failNExit(const char *msg);

typedef struct slave_info {
    pid_t pid;
    int pipe_father_to_child[2];
    int pipe_child_to_father[2];
    char *file_path;
    char md5[32]; // deberia ser 32 + 1 prob
} Tslave_info;

#define SLAVE_COUNT 5
#define READ 0
#define WRITE 1

int main(int argc, char *argv[]) {

    // Error in case of bad call
    if (argc == 1) {
        failNExit("Usage: md5sum <file1> <file2> ...");
    }

    char *slave_path = "./slave";
    char *slave_args[] = {slave_path, NULL};
    Tslave_info slaves[SLAVE_COUNT] = {0};
    // Initialize slaves
    for (int i = 0; i < SLAVE_COUNT; i++) {
        // Create pipes for each slave
        if (pipe(slaves[i].pipe_father_to_child) == -1) {
            failNExit("Error when initializing father to child pipe");
        }

        if (pipe(slaves[i].pipe_child_to_father) == -1) {
            failNExit("Error when initializing child to father pipe");
        }

        // Fork
        slaves[i].pid = fork();
        if (slaves[i].pid == -1) {
            failNExit("Error when initializing slave");
        }

        // Connect pipes between father and child
        if (slaves[i].pid == 0) {

            // Connect father to child pipe to child stdin
            if (close(slaves[i].pipe_father_to_child[WRITE]) == -1) {
                failNExit("Error when closing pipe write end");
            }

            if (dup2(slaves[i].pipe_father_to_child[READ], STDIN_FILENO) == -1) {
                failNExit("Error when duplicating pipe");
            }

            // Connect child to father pipe to child stdout
            if (close(slaves[i].pipe_child_to_father[WRITE]) == -1) {
                failNExit("Error when closing pipe read end");
            }

            if (dup2(slaves[i].pipe_child_to_father[WRITE], STDOUT_FILENO) == -1) {
                failNExit("Error when duplicating pipe");
            }

            if (execv(slave_path, slave_args) == -1) {
                failNExit("Error with execv");
            }
            // not necessary (execv takes over process)
            // return;
        } else { // choose what to do on parent
        }
    }

    // pass files to slaves
    char *current_file;
    for (int i = 1; i < argc; i++) {
        current_file = argv[i];
        // handle file in slave process
    }
    return 0;
}

static void failNExit(const char *msg) {
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}
