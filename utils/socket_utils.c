// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com 

#include "socket_utils.h"

int init_fd_set(fd_set* readfds, int fds[], unsigned int count) {
    FD_ZERO(readfds);
    int max_fd = -1;
    
    for (unsigned int i = 0; i < count; i++) {
        if (fds[i] != -1) {
            FD_SET(fds[i], readfds);
            if (fds[i] > max_fd) max_fd = fds[i];
        }
    }
    return max_fd;
}

int wait_for_fds(int max_fd, fd_set* readfds, unsigned int timeout) {
    if (max_fd == -1) return -2; // No active file descriptors
    
    struct timeval tv = {timeout, 0};
    int ready = select(max_fd + 1, readfds, NULL, NULL, &tv);
    
    if (ready < 0) {
        perror("select");
        return -1;
    }
    
    return ready; // Returns 0 on timeout
}
