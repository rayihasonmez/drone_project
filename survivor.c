#include "survivor.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "list.h"
#include <time.h>

Survivor *survivor_create(int id, float x, float y) {
    Survivor *s = malloc(sizeof(Survivor));
    s->id = id;
    s->x = x;
    s->y = y;
    s->created_at = time(NULL);
    s->rescued_at = 0;
    return s;
}

void survivor_destroy(void *s) {
    free(s);
}

// Random survivor generator thread
void *survivor_generator_thread(void *arg) {
    List *survivors = (List*)arg;
    int next_id = 1;
    while (1) {
        float x = rand() % 800;
        float y = rand() % 600;
        Survivor *s = survivor_create(next_id++, x, y);
        list_append(survivors, s);
        printf("Survivor generated: id=%d\n", s->id);
        sleep(rand()%4+2); // New survivor every 2~5 seconds
    }
    return NULL;
}