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
#include "weight.h"

using namespace std;


void initialize_processes(pid_t* pids) {
    for (int i = 0; i < BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i < BELT_COUNT) {
                //i * 3 + 2 porque i == 0: 2kg, i == 1 : 5kg
                belt_action(((i * 3) + 2), (i + 1));
            } else if (i < BELT_COUNT + DISPLAY_COUNT) {
                display_action();
            } else {
                weight_action();
            }
            exit(0);
        }
    }
}

int main(){
    pid_t pids[BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT];
    initialize_processes(pids);

    int status;
    waitpid(pids[BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT - 1], &status, WUNTRACED);

    return 0;
}