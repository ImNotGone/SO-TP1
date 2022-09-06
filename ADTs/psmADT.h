#ifndef _PSM_ADT_H
#define _PSM_ADT_H

#define _BSD_SOURCE // SOURCE: man ftruncate (docker)

#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_NAME_LEN 256

typedef struct psmCDT * psmADT;

// creates a new posix shared memory space
psmADT newPsm(const char * name, int oflag, mode_t mode);

// frees the posix shared memory space
void freePsm(psmADT psm);

#endif
