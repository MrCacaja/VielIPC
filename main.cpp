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


void initialize_processes() {
    for (int i = 0; i < BELT_COUNT + DISPLAY_COUNT; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i < BELT_COUNT) {
                //i * 3 + 2 porque i == 0: 2kg, i == 1 : 5kg
                belt_action(((i * 3) + 2), (i + 1));
            } else if (i < BELT_COUNT + DISPLAY_COUNT) {
                display_action();
            }
            exit(0);
        }
    }
}

int main(){
    initialize_processes();
    sleep(5);

    if (shm_unlink(MEMORY_NAME) == -1) {
        printf("Error unlinking");
        return -1;
    }

    return 0;
}