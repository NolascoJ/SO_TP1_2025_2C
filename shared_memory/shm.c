// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 

#define _XOPEN_SOURCE 500
#include "shm.h"


void* shm_init(const char *name, size_t size, int *shm_fd_out, int mode) {
    mode_t permissions = (mode == O_RDWR) ? 0666 : 0444;
    int shm_fd = shm_open(name, O_CREAT | mode, permissions);
    if (shm_fd == -1) {
        perror("shm_open (init)");
        return NULL;
    }
    
    if (mode == O_RDWR && ftruncate(shm_fd, size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        return NULL;
    }
    
    int prot = (mode == O_RDWR) ? (PROT_READ | PROT_WRITE) : PROT_READ;
    void* ptr = mmap(0, size, prot, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (init)");
        close(shm_fd);
        return NULL;
    }

    *shm_fd_out = shm_fd;
    return ptr;
}

void* shm_open_and_map(const char *name, size_t size, int *shm_fd_out, int mode) {
    int shm_fd = shm_open(name, mode, 0);
    if (shm_fd == -1) {
        perror("shm_open (open)");
        return NULL;
    }

    int prot = (mode == O_RDWR) ? (PROT_READ | PROT_WRITE) : PROT_READ;
    void* ptr = mmap(0, size, prot, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap (open)");
        close(shm_fd);
        return NULL;
    }

    *shm_fd_out = shm_fd;
    return ptr;
}

int shm_close(void* ptr, size_t size, int shm_fd) {
    if (munmap(ptr, size) == -1) {
        perror("munmap");
        return -1;
    }

    if (close(shm_fd) == -1) {
        perror("close");
        return -1;
    }

    return 0;
}

int shm_destroy(const char *name) {
    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
        return -1;
    }
    return 0;
}