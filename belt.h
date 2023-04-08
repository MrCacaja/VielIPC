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
#include <string.h>

void belt_start_write_shared_memory(int &shm_fd, void *&ptr) {
    //O_RDWR = 0010 < abre arquivo para leitura e escrita
    //O_CREAT = 0100 < permite criação inicial do arquivo
    shm_fd = shm_open(MEMORY_NAME,  O_RDWR | O_CREAT, MODE);
    if (shm_fd == -1) {
        printf("Esteira: memória compartilhada falhou\n");
        exit(-1);
    }

    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Esteira: Erro ao mapear memória\n");
        exit(-1);
    }
    ftruncate(shm_fd,SIZE);
    sleep(0.1);
}

void belt_action(int med_weight, int interval) {
    printf("belt %dKg | %ds interval\n", med_weight, interval);
    int shm_fd = 0;
    void *ptr;
    belt_start_write_shared_memory(shm_fd, ptr);
    sprintf((char*)ptr, "mandando mensagem");
    ptr = static_cast<char *>(ptr) + std::to_string(med_weight).length();
}

#endif //VIELIPC_BELT_H
