#ifndef DRONE_H
#define DRONE_H

#include <pthread.h>
#include <stdbool.h>

typedef enum { DRONE_IDLE, DRONE_MOVING, DRONE_ON_MISSION } DroneStatus;

typedef struct {
    int id;
    float x, y;
    DroneStatus status;
    pthread_t thread;
    int assigned_survivor_id; // -1 if none
} Drone;

Drone *drone_create(int id, float x, float y);
void drone_destroy(void *drone_ptr);
void *drone_thread_fn(void *arg);

#endif