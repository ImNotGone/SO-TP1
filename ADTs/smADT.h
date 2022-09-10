#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <unistd.h>


#define FILES_PER_SLAVE 2
#define SLAVE_PATH "./slave"


typedef struct smCDT * smADT;

smADT newSm(int fileQty, char **files);

int smHasFilesLeft(smADT sm);

ssize_t smRetrieve(smADT sm, char * buffer, int bufferSize);

void freeSm(smADT sm);
