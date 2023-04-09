#ifndef VIELIPC_DISPLAY_H
#define VIELIPC_DISPLAY_H

void display_start_write_pipe() {

}

void display_action_pipe() {

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
