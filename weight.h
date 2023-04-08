#ifndef VIELIPC_WEIGHT_H
#define VIELIPC_WEIGHT_H

void weight_start_write_shared_memory(int &shm_fd, void *&ptr) {
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

void weight_action() {
    int remaining_reruns = TOTAL_WEIGHT_RERUNS;
    int shm_fd = 0;
    int total_weight = 0;
    void *ptr = nullptr;
    weight_start_write_shared_memory(shm_fd, ptr);
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
