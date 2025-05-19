#include "ai.h"
#include "drone.h"
#include "list.h"
#include "survivor.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <unistd.h>

// Helper: Euclidean distance
static float dist(float x1, float y1, float x2, float y2) {
    float dx = x1 - x2, dy = y1 - y2;
    return sqrtf(dx*dx + dy*dy);
}

// Assign oldest survivor to nearest idle drone
void *ai_controller_thread(void *arg) {
    struct {
        List *drones;
        List *survivors;
    } *ctx = arg;

    while (1) {
        Drone *best_drone = NULL;
        Survivor *oldest_survivor = NULL;
        float best_dist = FLT_MAX;

        // Find oldest non-rescued survivor
        pthread_mutex_lock(&ctx->survivors->mutex);
        ListNode *sn = ctx->survivors->head;
        while (sn && ((Survivor*)sn->data)->rescued_at != 0) sn = sn->next;
        if (!sn) {
            pthread_mutex_unlock(&ctx->survivors->mutex);
            usleep(200*1000); continue;
        }
        oldest_survivor = (Survivor*)sn->data;
        pthread_mutex_unlock(&ctx->survivors->mutex);

        // Find nearest idle drone
        pthread_mutex_lock(&ctx->drones->mutex);
        for (ListNode *dn = ctx->drones->head; dn; dn = dn->next) {
            Drone *d = (Drone*)dn->data;
            if (d->status == DRONE_IDLE) {
                float d2 = dist(d->x, d->y, oldest_survivor->x, oldest_survivor->y);
                if (d2 < best_dist) {
                    best_dist = d2;
                    best_drone = d;
                }
            }
        }
        if (best_drone) {
            // Atama ve survivor'u "rescued" olarak işaretle
            best_drone->status = DRONE_ON_MISSION;
            best_drone->assigned_survivor_id = oldest_survivor->id;
            printf("AI: Drone %d -> Survivor %d (%.1f,%.1f)\n", best_drone->id, oldest_survivor->id, oldest_survivor->x, oldest_survivor->y);
            // Drone'u hedefe gönder (basitleştirilmiş, gerçek hareket yok)
            // Survivor'u hemen silmek yerine rescued_at ayarla
            pthread_mutex_lock(&ctx->survivors->mutex);
            oldest_survivor->rescued_at = time(NULL);
            pthread_mutex_unlock(&ctx->survivors->mutex);
        }
        pthread_mutex_unlock(&ctx->drones->mutex);

        usleep(200*1000); // 200ms döngü
    }
    return NULL;
}