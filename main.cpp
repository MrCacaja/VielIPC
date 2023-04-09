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

    while (true) {
        for (int i = 0; i < BELT_COUNT + DISPLAY_COUNT + WEIGHT_COUNT; i++) {
            int socket = newsockfd[i];
            // Read data from client
            memset(buffer, 0, sizeof buffer);
            if (read(socket, buffer, sizeof(buffer)) < 0) {
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
                    printf("Finalizado\n");
                    exit(0);
                } else if ((char)buffer[0] == RESTART[0]) {
                    weight_list = "";
                }
            }
        }
//
//        // Process data
//        // In this example, we just convert the string to uppercase
//        for (int i = 0; i < strlen(buffer); i++)
//        {
//            buffer[i] = toupper(buffer[i]);
//        }
//
//        // Write processed data back to client
//        if (write(newsockfd, buffer, strlen(buffer) + 1) < 0)
//        {
//            perror("Falha em escrever no socket");
//            close(newsockfd);
//            close(sockfd);
//            exit(1);
//        }
//
//        printf("Dado enviado de volta para o cliente.\n");
    }

    // Close sockets and exit
    for (int & i : newsockfd) close(i);
    close(sockfd);
    exit(0);
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