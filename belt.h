#ifndef VIELIPC_BELT_H
#define VIELIPC_BELT_H

#include "global.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "iostream"

void start_write_shared_memory(int &shm_fd, void *&ptr) {
    //O_CREAT = 0100 < cria arquivo se não existir
    //O_RDWR = 0010 < abre arquivo para leitura e escrita
    //O_CREAT | O_RDWR = 0110 < faz ambos acima
    //shm_fd < abre um segmento de memória com o nome escolhido (para sincronização), com as flags de leitura
    shm_fd = shm_open(MEMORY_NAME, O_CREAT | O_RDWR, MODE);
    if (shm_fd == -1) {
        printf("shared memory failed\n");
        exit(-1);
    }
    //Trunca o tamanho do arquivo no segmento
    ftruncate(shm_fd, SIZE);

    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Erro ao mapear memória\n");
        exit(-1);
    }
}

void belt_action() {
    int shm_fd = 0;
    void *ptr = nullptr;
    start_write_shared_memory(shm_fd, ptr);
    sprintf((char *)ptr, "dda");
}

#endif //VIELIPC_BELT_H
