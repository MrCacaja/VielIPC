#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "iostream"
#include "belt.h"
#include "display.h"
#include "weight.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <chrono>

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
    int sockfd, len;
    int newsockfd[BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT];
    struct sockaddr_un local, remote;
    char buffer[1];
    string weight_list;

    // Create socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Falha em criar o pipe");
        exit(1);
    }

    // Bind socket to local address
    memset(&local, 0, sizeof(local));
    local.sun_family = AF_UNIX;
    strncpy(local.sun_path, SOCKET_PATH, sizeof(local.sun_path) - 1);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(sockfd, (struct sockaddr *)&local, len) < 0)
    {
        perror("Falha em capturar o socket");
        close(sockfd);
        exit(1);
    }

    // Listen for connections
    if (listen(sockfd, 5) < 0) {
        perror("Falha em escutar o socket");
        close(sockfd);
        exit(1);
    }

    printf("Servidor Named pipe ouvindo em %s...\n", SOCKET_PATH);

    for (int i = 0; i < BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            if (i < BELT_COUNT) {
                //i * 3 + 2 porque i == 0: 2kg, i == 1 : 5kg
                belt_action_pipe(((i * 3) + 2), (i + 1));
            } else if (i < BELT_COUNT + DISPLAY_COUNT) {
                display_action_pipe();
            } else {
                weight_action_pipe();
            }
            exit(0);
        }
    }

    for (int & i : newsockfd) {
        memset(&remote, 0, sizeof(remote));
        len = sizeof(remote);
        i = accept(sockfd, (struct sockaddr *)&remote, (socklen_t*)&len);
        if (i < 0) {
            perror("Falha em aceitar coneccao");
            close(sockfd);
            exit(1);
        }
        //Configuração de timeout de leitura
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 1;
        setsockopt(i, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        printf("Cliente conectado!\n");
    }

    float lost_time = 0;
    int miss_count = 0;
    chrono::time_point begin = chrono::steady_clock::now();
    while (true) {
        for (int i = 0; i < BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT; i++) {
            int socket = newsockfd[i];
            // Read data from client
            memset(buffer, 0, sizeof buffer);
            if (read(socket, buffer, sizeof(buffer)) < 0) {
                if (lost_time == 0) {
                    chrono::time_point end = chrono::steady_clock::now();
                    lost_time = chrono::duration_cast<chrono::microseconds>(end - begin).count() / (float)1000000;
                }
                miss_count++;
                //Não reporta erro porque o timeout é considerado um erro, e apenas deve ser pulado
                continue;
            }

            if (!buffer[0]) {
                continue;
            }

            if (i < BELT_COUNT) {
                if ((char)buffer[0] == CHECK_PERMISSION[0]) {
                    if (weight_list.length() < 500) {
                        if (write(socket, "1", 1) < 0) {
                            perror("Falha em escrever no socket");
                            close(socket);
                            close(sockfd);
                            exit(1);
                        }
                    } else {
                        if (write(socket, "0", 1) < 0) {
                            perror("Falha em escrever no socket");
                            close(socket);
                            close(sockfd);
                            exit(1);
                        }
                    }
                } else {
                    weight_list.push_back(buffer[0]);
                }
            } else if (i < BELT_COUNT + DISPLAY_COUNT) {
                if ((char)buffer[0] == GET_ITEMS[0]) {
                    if (write(socket, weight_list.c_str(), SIZE) < 0) {
                        perror("Falha em escrever no socket");
                        close(socket);
                        close(sockfd);
                        exit(1);
                    }
                }
            } else {
                if ((char)buffer[0] == GET_ITEMS[0]) {
                    if (write(socket, weight_list.c_str(), SIZE) < 0) {
                        perror("Falha em escrever no socket");
                        close(socket);
                        close(sockfd);
                        exit(1);
                    }
                } else if ((char)buffer[0] == KILL[0]) {
                    for (int p = 0; p < WEIGHT_COUNT + DISPLAY_COUNT + BELT_COUNT; p++) {
                        kill(pids[p], SIGTERM);
                    }
                    chrono::time_point end = chrono::steady_clock::now();
                    printf(
                            "Finalizado em %fs, com %d intruções vazias no servidor, sendo %fs o tempo médio para não retornar mensagens",
                            chrono::duration_cast<chrono::microseconds>(end - begin).count() / (float)1000000,
                            miss_count,
                            lost_time
                            );
                    exit(0);
                } else if ((char)buffer[0] == RESTART[0]) {
                    weight_list = "";
                }
            }
        }
    }
}

void initialize_processes_thread() {
    printf("Rodando com threads\n");
    pthread_t threads[BELT_COUNT + WEIGHT_COUNT + DISPLAY_COUNT];

    chrono::time_point begin = chrono::steady_clock::now();
    for (int i = 0; i < BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT; i++) {
        if (i < BELT_COUNT) {
            struct belt_thread_args *args = (struct belt_thread_args *)malloc(sizeof(struct belt_thread_args));
            //i * 3 + 2 porque i == 0: 2kg, i == 1 : 5kg
            args->med_weight = (i * 3) + 2;
            args->interval = i + 1;
            pthread_create(&threads[i], NULL, belt_thread, (void*)args);
        } else if (i < BELT_COUNT + DISPLAY_COUNT) {
            pthread_create(&threads[i], NULL, display_thread, NULL);
        } else {
            pthread_create(&threads[i], NULL, weight_thread, NULL);
        }
    }

    pthread_join(threads[BELT_COUNT + WEIGHT_COUNT + DISPLAY_COUNT - 1], NULL);
    chrono::time_point end = chrono::steady_clock::now();
    printf("Finalizado em %f segundos", chrono::duration_cast<chrono::microseconds>(end - begin).count() / (float)1000000);
}

int main(){
    pid_t pids[BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT];
    if (IPC == 0){
        chrono::time_point begin = chrono::steady_clock::now();
        initialize_processes_shm(pids);
        int status;
        waitpid(pids[BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT - 1], &status, WUNTRACED);
        for (int pid : pids) {
            kill(pid, SIGTERM);
        }
        chrono::time_point end = chrono::steady_clock::now();
        printf("Finalizado com status %d em %f segundos", status, chrono::duration_cast<chrono::microseconds>(end - begin).count() / (float)1000000);
    } else if (IPC == 1) {
        initialize_processes_pipe(pids);
    } else {
        initialize_processes_thread();
    }

    return 0;
}