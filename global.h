#ifndef VIELIPC_GLOBAL_H
#define VIELIPC_GLOBAL_H

#define SIZE 4096
//0666 is the usual access permision in linux in rwx octal format and having the sequence(owner-group-user).
#define MODE 0666
#define MEMORY_NAME "FOOD_SEC"

// 0 e 1 - esteiras
#define BELT_COUNT 2
// 2 - display
#define DISPLAY_COUNT 1

pid_t pids[3];

#endif //VIELIPC_GLOBAL_H
