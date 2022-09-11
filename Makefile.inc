COMP=gcc
COMP_FLAGS=-Wall -std=c99 -pedantic
BINARIES=md5 slave view testwrite testread
MD5_LOG_FILE=md5.log
PVS_OUTPUT=PVS-Studio.log report.tasks strace_out
CREDENTIALS=.config/PVS-Studio/PVS-Studio.lic
LD=gcc
LD_FLAGS=-Wall -std=c99 -pedantic -lrt -lpthread
