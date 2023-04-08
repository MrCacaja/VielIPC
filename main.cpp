#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "iostream"
#include "belt.h"
#include "display.h"
#include "weight.h"
#include <sys/socket.h>
#include <sys/un.h>

using namespace std;


void initialize_processes_shm(pid_t* pids) {
    printf("Rodando com memórida compartilhada\n");
    for (int i = 0; i < BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i < BELT_COUNT) {
                //i * 3 + 2 porque i == 0: 2kg, i == 1 : 5kg
                belt_action_shm(((i * 3) + 2), (i + 1));
            } else if (i < BELT_COUNT + DISPLAY_COUNT) {
                display_action_shm();
            } else {
                weight_action_shm();
            }
            exit(0);
        }
    }
}

void initialize_processes_pipe(pid_t* pids) {
    printf("Rodando com pipe\n");
}

void initialize_processes_thread() {
    printf("Rodando com threads\n");
}

int main(){
    pid_t pids[BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT];
    if (IPC == 0){
        initialize_processes_shm(pids);
        int status;
        waitpid(pids[BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT - 1], &status, WUNTRACED);
        for (int pid : pids) {
            kill(pid, SIGTERM);
        }
        printf("Finalizado com status %d", status);
    } else if (IPC == 1) {
        initialize_processes_pipe(pids);
    } else {
        initialize_processes_thread();
    }

    return 0;
}