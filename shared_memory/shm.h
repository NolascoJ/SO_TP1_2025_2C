#ifndef shm_H
#define shm_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

void* shm_init(const char *name, size_t size, int *shm_fd, int mode);

void* shm_open_and_map(const char *name, size_t size, int *shm_fd, int mode);

int shm_close(void* ptr, size_t size, int shm_fd);

int shm_destroy(const char *name);

#endif