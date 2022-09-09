// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _GNU_SOURCE // SOURCE: man getline
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_CMD_LEN 2048
#define MD5_OUT_LEN 32
#define CMD_FMT_STR "md5sum %s"
#define OUT_FMT_STR "File: %s, PID: %d, md5: %s\n"

void formatOutput(char *output, FILE *stream);
static void solve(char *filename);
static int checkPath(char *path);
static void failNExit(const char *msg);

// Filename read from stdin
// Filename, PID & md5sum get writen to stdout
int main(int argc, char *argv[]) {

    // === getline ===
    char *filename = NULL;
    size_t len = 0;
    ssize_t nread;
    // ===

    setvbuf(stdout, NULL, _IONBF, 0);

    // Read filename from stdin
    while ((nread = getline(&filename, &len, stdin)) > 0) {
        // Remove the '\n'
        filename[nread - 1] = '\0';
        solve(filename);
    }

    // Catch getline error if ocurred
    if (errno == EINVAL || errno == ENOMEM) {
        failNExit("getline");
    }

    free(filename); // SOURCE: man getline
    return EXIT_SUCCESS;
}

// Copies stream to output
void formatOutput(char *output, FILE *stream) {
    char c;
    int i = 0;

    while (i < MD5_OUT_LEN && (c = getc(stream)) > 0) {
        output[i++] = c;
    }
    output[i] = '\0';
}

static void failNExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

static void solve(char *filename) {

    if (!checkPath(filename)) {
        fprintf(stdout, "%s is not a file\n", filename);
        return;
    }
    // === cmd ===
    char cmd_buff[MAX_CMD_LEN] = {0};       // command to execute
    char md5_output[MD5_OUT_LEN + 1] = {0}; // command output
    // ===

    // build the command to execute by
    // printing the command to cmd_buff
    if (sprintf(cmd_buff, CMD_FMT_STR, filename) < 0)
        failNExit("Error in sprintf");

    // execute the command
    FILE *commandOutput = popen(cmd_buff, "r");
    if (commandOutput == NULL)
        failNExit("popen function error");

    // catch the output -> md5_output
    formatOutput(md5_output, commandOutput);

    // format output for stdout
    fprintf(stdout, OUT_FMT_STR, filename, getpid(), md5_output);
    if (pclose(commandOutput) == -1)
        failNExit("pclose function error");

    // TODO: cleanup
}

static int checkPath(char *path) {
    struct stat validation;
    return stat(path, &validation) >= 0 && S_ISREG(validation.st_mode);
}
