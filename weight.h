#ifndef VIELIPC_WEIGHT_H
#define VIELIPC_WEIGHT_H

void* weight_thread(void*) {
    int remaining_reruns = TOTAL_WEIGHT_RERUNS;
    int total_weight = 0;
    while (true) {
        pthread_mutex_lock(&item_mutex);
        if (items.length() >= SIZE) {
            int weight = 0;
            for (char item : items) {
                weight += item - 48;
            }
            total_weight += weight;
            printf("Peso: %dkg\n", weight);
            printf("Peso total: %dkg\n", total_weight);
            remaining_reruns--;
            if (remaining_reruns < 0){
                break;
            } else {
                items = "";
            }
        }
        pthread_mutex_unlock(&item_mutex);
        usleep(1000000 * WEIGHT_INTERVAL / INTERVAL_DIVIDER);
    }
}

void weight_start_write_pipe(int &sockfd) {
    int len;
    struct sockaddr_un remote;

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Balança: Falha em criar o socket");
        exit(1);
    }

    // Connect to server
    memset(&remote, 0, sizeof(remote));
    remote.sun_family = AF_UNIX;
    strncpy(remote.sun_path, SOCKET_PATH, sizeof(remote.sun_path) - 1);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(sockfd, (struct sockaddr *)&remote, len) < 0)
    {
        perror("Balança: Falha em conectar no servidor");
        close(sockfd);
        exit(1);
    }

    printf("Balança conectada ao computador!\n");
}

void weight_action_pipe() {
    int remaining_reruns = TOTAL_WEIGHT_RERUNS;
    int total_weight = 0;
    char buffer[SIZE];
    int sockfd = 0;

    weight_start_write_pipe(sockfd);

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

        if (strlen(buffer) >= SIZE) {
            int weight = 0;
            for (int i = 0; i < strlen(buffer); i++) {
                weight += ((buffer)[i] - 48);
            }
            total_weight += weight;
            printf("Peso: %dkg\n", weight);
            printf("Peso total: %dkg\n", total_weight);
            remaining_reruns--;
            if (remaining_reruns < 0){
                if (write(sockfd, KILL, 1) < 0) {
                    perror("Display: Falha em escrever no socket");
                    close(sockfd);
                    exit(1);
                }
            } else if (write(sockfd, RESTART, 1) < 0) {
                perror("Display: Falha em escrever no socket");
                close(sockfd);
                exit(1);
            }
        }

        usleep(1000000 * WEIGHT_INTERVAL / INTERVAL_DIVIDER);
    }
}

void weight_start_write_shared_memory_shm(int &shm_fd, void *&ptr) {
    usleep(100000 * BELT_WAITER);

    //O_RDWR = 0010 < abre arquivo para leitura e escritura
    shm_fd = shm_open(MEMORY_NAME, O_RDWR, MODE);
    if (shm_fd == -1) {
        printf("Balança: memória compartilhada falhou\n");
        exit(-1);
    }

    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Balança: Erro ao mapear memória\n");
        exit(-1);
    }
}

void weight_action_shm() {
    int remaining_reruns = TOTAL_WEIGHT_RERUNS;
    int shm_fd = 0;
    int total_weight = 0;
    void *ptr = nullptr;
    weight_start_write_shared_memory_shm(shm_fd, ptr);
    while (true) {
        if (strlen((char*)ptr) >= SIZE) {
            int weight = 0;
            for (int i = 0; i < strlen((char*)ptr); i++) {
                weight += (((char*)ptr)[i] - 48);
            }
            total_weight += weight;
            printf("Peso: %dkg\n", weight);
            printf("Peso total: %dkg\n", total_weight);
            remaining_reruns--;
            if (remaining_reruns < 0){
                exit(0);
            } else {
                sprintf((char*)ptr, "");
            }
        }
        usleep(1000000 * WEIGHT_INTERVAL / INTERVAL_DIVIDER);
    }
}

#endif //VIELIPC_WEIGHT_H
