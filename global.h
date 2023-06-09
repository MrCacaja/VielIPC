#ifndef VIELIPC_GLOBAL_H
#define VIELIPC_GLOBAL_H

// 0 - Memória compartilhada, 1 - Pipe, 2 - Thread
#define IPC 0

#define SIZE 500
//0666 is the usual access permision in linux in rwx octal format and having the sequence(owner-group-user).
#define MODE 0666
#define MEMORY_NAME "FOOD_SEC"

#define SOCKET_PATH "/tmp/food_sec"

// 0 e 1 - esteiras
#define BELT_COUNT 2
// 2 - display
#define DISPLAY_COUNT 1
// 3 - balança
#define WEIGHT_COUNT 1

#define INTERVAL_DIVIDER 100
#define WEIGHT_INTERVAL 5
#define DISPLAY_INTERVAL 2
#define BELT_INTERVAL 2
#define BELT_WAITER 3

#define TOTAL_WEIGHT_RERUNS 0

#define LOCKED_MESSAGE "0"
#define CHECK_PERMISSION "C"
#define GET_ITEMS "G"
#define KILL "K"
#define RESTART "R"

//Variáveis multi-thread:
pthread_mutex_t item_mutex = PTHREAD_MUTEX_INITIALIZER;
std::string items = "";

#endif //VIELIPC_GLOBAL_H
