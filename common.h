#ifndef COMMON_H
#define COMMON_H

typedef struct {
    int x, y;
    int helped; // 0: bekliyor, 1: yardım edildi
} Survivor;

#define DRONE_ID_LEN 32
typedef struct {
    char id[DRONE_ID_LEN];
    int x, y;
    char status[16];
    int fd;
} DroneInfo;

#endif