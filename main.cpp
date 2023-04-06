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
#include "belt.h"
#include "display.h"

using namespace std;


void initialize_processes(int quantity) {
    for (int i = 0; i < quantity; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i < BELT_COUNT) {
                belt_action();
            } else if (i < BELT_COUNT + DISPLAY_COUNT) {
                display_action();
            }
            exit(0);
        }
    }
}

int main(){
    int shm_fd;
    void *ptr;

    initialize_processes(5);
    return 1;

    //O_RDONLY = 0000 < apenas leitura
    shm_fd = shm_open(MEMORY_NAME, O_RDONLY, MODE);
    if (shm_fd == -1) {
        printf("shared memory failed\n");
        return -1;
    }
    ftruncate(shm_fd, SIZE);

    ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED) {
        printf("Erro ao mapear memória\n");
        return -1;
    }

    printf("%s", (char *)ptr);

    //initialize_processes(5);

    if (shm_unlink(MEMORY_NAME) == -1) {
        printf("Error unlinking");
        return -1;
    }

    return 0;
}