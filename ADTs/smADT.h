#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>

typedef struct smCDT * smADT;

smADT newSm(int fileQty, char **files, int minFilesPerSlave, const char *slavePath);

int smHasFilesLeft(smADT sm);

ssize_t smRetrieve(smADT sm, char * buffer, int bufferSize);

void freeSm(smADT sm);
