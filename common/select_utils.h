#ifndef SELECT_UTILS_H
#define SELECT_UTILS_H

#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>

int init_fd_set(fd_set* readfds, int fds[], unsigned int count);
int wait_for_fds(int max_fd, fd_set* readfds, unsigned int timeout);

#endif // SELECT_UTILS_H
