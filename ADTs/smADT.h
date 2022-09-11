#ifndef _SM_ADT_H
#define _SM_ADT_H

#define _GNU_SOURCE
#include <sys/select.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define SLAVE_PATH "./slave"
#define MAX_FILES_PER_SLAVE 4

typedef struct smCDT * smADT;

smADT newSm(int fileQty, char ** files);

int addSlave();

int hasNextFile(smADT sm);

ssize_t smRead(smADT sm, char * buff, size_t bytes);

void freeSm(smADT sm);

#endif