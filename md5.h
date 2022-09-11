#include "ADTs/pshmADT.h"
#include "ADTs/smADT.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define MIN_FILES_PER_SLAVE 2
