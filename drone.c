#include "drone.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

Drone *drone_create(int id, float x, float y) {
    Drone *d = malloc(sizeof(Drone));
    d->id = id;
    d->x = x;
    d->y = y;
    d->status = DRONE_IDLE;
    d->assigned_survivor_id = -1;
    return d;
}

void drone_destroy(void *drone_ptr) {
    free(drone_ptr);
}

void *drone_thread_fn(void *arg) {
    Drone *d = (Drone*)arg;
    while (1) {
        if (d->status == DRONE_ON_MISSION) {
            // Simulate moving to survivor (sleep)
            printf("Drone %d rescuing survivor %d\n", d->id, d->assigned_survivor_id);
            sleep(2);
            d->status = DRONE_IDLE;
            d->assigned_survivor_id = -1;
        }
        usleep(100*1000); // 100ms loop
    }
    return NULL;
}