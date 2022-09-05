#include "md5.h"

typedef struct slave_info {
    pid_t pid;
    int pipe_father_to_child[2];
    int pipe_child_to_father[2];
    char *file_path;
    char md5[32]; // deberia ser 32 + 1 prob
} Tslave_info;

#define SLAVE_COUNT 5
#define READ 1
#define WRITE 0

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
            if (close(slaves[i].pipe_child_to_father[READ]) == -1) {
                failNExit("Error when closing pipe read end");
            }

            if (dup2(slaves[i].pipe_child_to_father[WRITE], STDOUT_FILENO) == -1) {
                failNExit("Error when duplicating pipe");
            }

            execv(slave_path, slave_args);

            // If execv fails, it returns and we exit
            failNExit("Error with execv");

        } else if (slaves[i].pid < 0) {

            failNExit("Error when using fork");
        }
        // Parent
        // Close unused pipe ends
        if (close(slaves[i].pipe_father_to_child[READ]) == -1) {
            failNExit("Error when closing pipe read end");
        }

        if (close(slaves[i].pipe_child_to_father[WRITE]) == -1) {
            failNExit("Error when closing pipe write end");
        }
    }

    // pass files to slaves
    char *current_file;
    for (int i = 1; i < argc; i++) {
        current_file = argv[i];

        if (access(current_file, F_OK) == -1) {
            // TODO: handle error (crash or continue?)
        } else {
            // handle file in slave process
        }
    }
    return 0;
}

static void failNExit(const char *msg) {
    fprintf(stderr, "%s", msg);
    exit(EXIT_FAILURE);
}
