// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "slave.h"

#define MAX_CMD_LEN 2048
#define MD5_OUT_LEN 32
#define CMD_FMT_STR "md5sum %s"
// %7d was chosen based on running this command
// cat /proc/sys/kernel/pid_max => 4194304 (7 digit number)
#define OUT_FMT_STR "PID: %7d, md5: %-32s, File: %s\n"
#define NAF_ERR_STR "[ERROR] NOT A FILE"

static void formatOutput(char *output, FILE *stream);
static void solve(char *filename);
static int checkPath(char *path);
static void failNExit(const char *msg);

// Filename read from stdin
// Filename, PID & md5sum get writen to stdout
int main(int argc, char *argv[]) {

    setvbuf(stdout, NULL, _IONBF, 0);

    // === getline ===
    char *filename = NULL;
    size_t len = 0;
    ssize_t nread;

    // Read filename from stdin
    while ((nread = getline(&filename, &len, stdin)) > 0) {
        // Remove the '\n'
        filename[nread - 1] = '\0';
        solve(filename);
    }

    // Catch getline error if ocurred
    if (errno == EINVAL || errno == EOVERFLOW || errno == ENOMEM) {
        failNExit("Error reading from shared memory");
    }

    // Free resources, getline uses malloc
    free(filename);
    return EXIT_SUCCESS;
}

// Copies stream to output
static void formatOutput(char *output, FILE *stream) {
    char c;
    int i = 0;

    while (i < MD5_OUT_LEN && (c = getc(stream)) > 0) {
        output[i++] = c;
    }
    output[i] = '\0';
}

static void solve(char *filename) {

    if (!checkPath(filename)) {
        fprintf(stdout, OUT_FMT_STR, getpid(), NAF_ERR_STR, filename);
        return;
    }
    // === cmd ===
    char cmd_buff[MAX_CMD_LEN] = {0};       // command to execute
    char md5_output[MD5_OUT_LEN + 1] = {0}; // command output
    // ===

    // build the command to execute by
    // printing the command to cmd_buff
    if (sprintf(cmd_buff, CMD_FMT_STR, filename) < 0) {
        failNExit("Error in sprintf");
    }

    // execute the command
    FILE *commandOutput = popen(cmd_buff, "r");
    if (commandOutput == NULL) {
        failNExit("popen function error");
    }

    // catch the output -> md5_output
    formatOutput(md5_output, commandOutput);

    // format output for stdout
    fprintf(stdout, OUT_FMT_STR, getpid(), md5_output, filename);
    if (pclose(commandOutput) == -1) {
        failNExit("pclose function error");
    }
}

static int checkPath(char *path) {
    struct stat validation;
    return stat(path, &validation) >= 0 && S_ISREG(validation.st_mode);
}

static void failNExit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}
