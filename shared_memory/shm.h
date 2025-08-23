#ifndef shm_H
#define shm_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>

// Inicializa la memoria compartida: la crea, le da un tamaño y la mapea.
// Retorna un puntero al segmento de memoria o NULL en caso de error.
void* shm_init(const char *name, size_t size, int *shm_fd, int mode);

// Abre y mapea una memoria compartida existente.
// Retorna un puntero al segmento de memoria o NULL en caso de error.
void* shm_open_and_map(const char *name, size_t size, int *shm_fd, int mode);

// Desmapea y cierra la memoria compartida.
// Retorna 0 en caso de éxito, -1 si falla.
int shm_close(void* ptr, size_t size, int shm_fd);

// Desvincula (elimina) la memoria compartida.
// Retorna 0 en caso de éxito, -1 si falla.
int shm_destroy(const char *name);

#endif