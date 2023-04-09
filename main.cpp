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
    int sockfd, newsockfd, len;
    struct sockaddr_un local, remote;
    char buffer[1024];
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
    if (listen(sockfd, 5) < 0)
    {
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

    while (true) {
        // Accept connections
        memset(&remote, 0, sizeof(remote));
        len = sizeof(remote);
        newsockfd = accept(sockfd, (struct sockaddr *)&remote, (socklen_t*)&len);
        if (newsockfd < 0)
        {
            perror("Falha em aceitar coneccao");
            close(sockfd);
            exit(1);
        }

        printf("Cliente conectado!\n");

        memset(buffer, 0, sizeof buffer);
        // Read data from client
        if (read(newsockfd, buffer, sizeof(buffer)) < 0)
        {
            perror("Falha em ler do socket");
            close(newsockfd);
            close(sockfd);
            exit(1);
        }

        if (!buffer[0]) {
            continue;
        }

        if ((char)buffer[0] == BELT_MESSAGE[0]) {
            printf("Dado recebido de esteira: %s\n", buffer);
            if (weight_list.length() < 500) {
                weight_list.append(to_string(buffer[1]));
                if (write(newsockfd, "1", 1) < 0) {
                    perror("Falha em escrever no socket");
                    close(newsockfd);
                    close(sockfd);
                    exit(1);
                }
            } else {
                weight_list.append(to_string(buffer[1]));
                if (write(newsockfd, "0", 1) < 0) {
                    perror("Falha em escrever no socket");
                    close(newsockfd);
                    close(sockfd);
                    exit(1);
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
    close(newsockfd);
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