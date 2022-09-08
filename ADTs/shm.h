#define _XOPEN_SOURCE 500
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BLOCK_SIZE 2048
#define SHM_ERROR -1

char * map_shm(char * name, size_t size, bool creator);
bool unmap_shm(char * addr, size_t size);
ssize_t write_shm(char * addr, char * buff, size_t bytes);
ssize_t read_shm(char * addr, char * buff, size_t bytes);
bool destroy_shm(char * name);
