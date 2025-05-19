#ifndef SURVIVOR_H
#define SURVIVOR_H

#include <pthread.h>
#include <time.h>

typedef struct {
    int id;
    float x, y;
    time_t created_at;
    time_t rescued_at; // 0 ise aktif, >0 ise kurtarıldı ve fade sürecinde
} Survivor;

void *survivor_generator_thread(void *arg);
Survivor *survivor_create(int id, float x, float y);
void survivor_destroy(void *s);

#endif