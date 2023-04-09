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
#include <sys/socket.h>
#include <sys/un.h>

struct belt_thread_args {
    int med_weight;
    float interval;
};

void* belt_thread(void* args) {
    while (true) {
        belt_thread_args *arg_ref = ((struct belt_thread_args*)args);
        pthread_mutex_lock(&item_mutex);
        if (items.length() < 500) items.push_back(48 + arg_ref->med_weight);
        pthread_mutex_unlock(&item_mutex);
        usleep(arg_ref->interval * 1000000 / INTERVAL_DIVIDER);
    }
}

void belt_start_write_pipe(int &sockfd) {
    int len;
    struct sockaddr_un remote;

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Esteira: Falha em criar o socket");
        exit(1);
    }

    // Connect to server
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCKET_PATH, sizeof(remote.sun_path) - 1);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sockfd, (struct sockaddr *)&remote, len) < 0)
    {
        perror("Falha em conectar no servidor");
        close(sockfd);
        exit(1);
    }

    printf("Esteira conectada ao computador!\n");
}

void belt_action_pipe(int med_weight, float interval) {
    std::string send_buffer;
    char buffer[1];
    int sockfd = 0;
    bool first_check;
    belt_start_write_pipe(sockfd);

    while (true) {
        send_buffer = "";
        first_check = true;
        do {
            send_buffer.push_back(CHECK_PERMISSION[0]);
            if (write(sockfd, send_buffer.c_str(), 1) < 0) {
                perror("Esteira: Falha em escrever no socket");
                close(sockfd);
                exit(1);
            }
            memset(buffer, 0, sizeof buffer);
            if (read(sockfd, buffer, sizeof(buffer)) < 0){
                perror("Esteira: Falha em ler do socket");
                close(sockfd);
                exit(1);
            }
            if (!first_check) {
                usleep(BELT_INTERVAL * 1000000 / INTERVAL_DIVIDER);
            } else {
                first_check = false;
            }
        } while (buffer[0] == LOCKED_MESSAGE[0]);
        send_buffer = "";
        send_buffer.push_back(char(48 + med_weight));
        if (write(sockfd, send_buffer.c_str(), 1) < 0) {
            perror("Esteira: Falha em escrever no socket");
            close(sockfd);
            exit(1);
        }
        usleep(interval * 1000000 / INTERVAL_DIVIDER);
    }
}

void belt_start_write_shared_memory_shm(int &shm_fd, void *&ptr) {
    //O_RDWR = 0010 < abre arquivo para leitura e escrita
    //O_CREAT = 0100 < permite criação inicial do arquivo
    shm_fd = shm_open(MEMORY_NAME,  O_RDWR | O_CREAT, MODE);
    if (shm_fd == -1) {
        printf("Esteira: memória compartilhada falhou\n");
        exit(-1);
    }
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    ftruncate(shm_fd,SIZE);
    sprintf((char*)ptr, ""); //Limpa memória
}

void belt_action_shm(int med_weight, float interval) {
    printf("belt %dKg | %fs interval\n", med_weight, interval);
    int shm_fd = 0;
    void *ptr;
    belt_start_write_shared_memory_shm(shm_fd, ptr);
    while (true) {
        while (strlen((char*)ptr) < SIZE) {
            sprintf((char*)ptr, "%s%d", (char*)ptr, med_weight);
            usleep(interval * 1000000 / INTERVAL_DIVIDER);
        }
        usleep(BELT_INTERVAL * 1000000 / INTERVAL_DIVIDER);
    }
}

#endif //VIELIPC_BELT_H
