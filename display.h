#ifndef VIELIPC_DISPLAY_H
#define VIELIPC_DISPLAY_H

void display_start_write_shared_memory(int &shm_fd, void *&ptr) {
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

void display_action() {
    printf("display\n");
    int shm_fd = 0;
    void *ptr = nullptr;
    display_start_write_shared_memory(shm_fd, ptr);
    while (true) {
        printf("%s\n", (char*)ptr);
        sleep(1);
    }
}

#endif //VIELIPC_DISPLAY_H
