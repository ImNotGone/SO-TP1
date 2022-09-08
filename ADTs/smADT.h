#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>

typedef struct smCDT * smADT;

smADT newSm();

int addSlave();

ssize_t smRead(smADT sm, char * buff, size_t bytes);

void freeSm(smADT sm);
