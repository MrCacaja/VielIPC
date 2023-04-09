#ifndef VIELIPC_DISPLAY_H
#define VIELIPC_DISPLAY_H

void* display_thread(void* args) {
    while (true) {
        pthread_mutex_lock(&item_mutex);
        printf("%zu\n", items.length());
        pthread_mutex_unlock(&item_mutex);
        usleep(1000000 * DISPLAY_INTERVAL / INTERVAL_DIVIDER);
    }
}

void display_start_write_pipe(int &sockfd) {
    int len;
    struct sockaddr_un remote;

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Display: Falha em criar o socket");
        exit(1);
    }

    // Connect to server
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCKET_PATH, sizeof(remote.sun_path) - 1);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sockfd, (struct sockaddr *)&remote, len) < 0)
    {
        perror("Display: Falha em conectar no servidor");
        close(sockfd);
        exit(1);
    }

    printf("Display conectado ao computador!\n");
}

void display_action_pipe() {
    char buffer[SIZE];
    int sockfd = 0;
    display_start_write_pipe(sockfd);

    while (true) {
        if (write(sockfd, GET_ITEMS, 1) < 0) {
            perror("Display: Falha em escrever no socket");
            close(sockfd);
            exit(1);
        }
        memset(buffer, 0, sizeof buffer);
        if (read(sockfd, buffer, sizeof(buffer)) < 0){
            perror("Display: Falha em ler do socket");
            close(sockfd);
            exit(1);
        }
        //Buffer vem com caracteres adicionais quando completo, considerar a contagem de itens máxima -2
        printf("Contagem de itens: %lu\n", strlen(buffer));
        usleep(1000000 * DISPLAY_INTERVAL / INTERVAL_DIVIDER);
    }
}

void display_start_write_shared_memory_shm(int &shm_fd, void *&ptr) {
    usleep(100000 * BELT_WAITER);

    //O_RDONLY = 0000 < abre arquivo para leitura
    shm_fd = shm_open(MEMORY_NAME, O_RDONLY, MODE);
    if (shm_fd == -1) {
        printf("Display: memória compartilhada falhou\n");
        exit(-1);
    }

    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Display: Erro ao mapear memória\n");
        exit(-1);
    }
}

void display_action_shm() {
    printf("display\n");
    size_t total = 0;
    int shm_fd = 0;
    void *ptr = nullptr;
    display_start_write_shared_memory_shm(shm_fd, ptr);
    while (true) {
        if (total != strlen((char*)ptr)) {
            total = strlen((char*)ptr);
            printf("%zu\n", total);
        }
        usleep(1000000 * DISPLAY_INTERVAL / INTERVAL_DIVIDER);
    }
}

#endif //VIELIPC_DISPLAY_H
