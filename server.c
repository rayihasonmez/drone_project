#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <time.h>
#include <sys/select.h>

void assign_missions();
void view_init();
void view_update(Survivor*, int, DroneInfo*, int);
void view_quit();

// typedef struct {
//     int x, y;
//     int helped; // 0: bekliyor, 1: yardım edildi
// } Survivor;

#define MAX_SURVIVORS 100
Survivor survivor_list[MAX_SURVIVORS];
int survivor_count = 0;
pthread_mutex_t survivor_list_mutex = PTHREAD_MUTEX_INITIALIZER;

// typedef struct {
//     char id[32];
//     int x, y;
//     char status[16];
//     int fd;
// } DroneInfo;

#define MAX_DRONES 100
DroneInfo drone_list[MAX_DRONES];
int drone_count = 0;
pthread_mutex_t drone_list_mutex = PTHREAD_MUTEX_INITIALIZER;

#define SERVER_PORT 12345

// void* view_thread_func(void* arg) {
//     (void)arg;
//     while (1) {
//         pthread_mutex_lock(&survivor_list_mutex);
//         pthread_mutex_lock(&drone_list_mutex);
//         view_update(survivor_list, survivor_count, drone_list, drone_count);
//         pthread_mutex_unlock(&drone_list_mutex);
//         pthread_mutex_unlock(&survivor_list_mutex);
//         SDL_Delay(50); // 20 FPS
//     }
//     return NULL;
// }
void* survivor_generator(void* arg) {
    while (1) {
        pthread_mutex_lock(&survivor_list_mutex);
        if (survivor_count < MAX_SURVIVORS) {
            survivor_list[survivor_count].x = rand() % 80 + 5;
            survivor_list[survivor_count].y = rand() % 80 + 5;
            survivor_list[survivor_count].helped = 0;
            survivor_count++;
        }
        pthread_mutex_unlock(&survivor_list_mutex);
        sleep(5); // Her 5 saniyede bir yeni survivor
    }
    return NULL;
}
void* handle_drone(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);
    char buffer[1024];

    while (1) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) break; // bağlantı koptu
        buffer[n] = '\0';
        printf("Gelen mesaj: %s\n", buffer);

        struct json_object *jobj = json_tokener_parse(buffer);
        if (jobj) {
            const char *drone_id = json_object_get_string(json_object_object_get(jobj, "drone_id"));
            struct json_object *loc = json_object_object_get(jobj, "location");
            int x = json_object_get_int(json_object_array_get_idx(loc, 0));
            int y = json_object_get_int(json_object_array_get_idx(loc, 1));
            const char *status = json_object_get_string(json_object_object_get(jobj, "status"));

            pthread_mutex_lock(&drone_list_mutex);
int found = 0;
for (int i = 0; i < drone_count; ++i) {
    if (strcmp(drone_list[i].id, drone_id) == 0) {
        drone_list[i].x = x;
        drone_list[i].y = y;
        strncpy(drone_list[i].status, status, sizeof(drone_list[i].status));
        found = 1;
        break;
    }
}
if (!found && drone_count < MAX_DRONES) {
    strncpy(drone_list[drone_count].id, drone_id, DRONE_ID_LEN-1);
    drone_list[drone_count].id[DRONE_ID_LEN-1] = '\0';
    drone_list[drone_count].x = x;
    drone_list[drone_count].y = y;
    strncpy(drone_list[drone_count].status, status, sizeof(drone_list[drone_count].status));
    drone_list[drone_count].fd = client_fd;
    drone_count++;
    assign_missions();
}
pthread_mutex_unlock(&drone_list_mutex);
            printf("---- Drone Listesi ----\n");
for (int i = 0; i < drone_count; ++i) {
    printf("ID: %s | Konum: (%d, %d) | Durum: %s\n",
        drone_list[i].id, drone_list[i].x, drone_list[i].y, drone_list[i].status);
}
printf("-----------------------\n");
            // pthread_mutex_unlock(&drone_list_mutex);

            // Survivor listesini de yazdırmak istersen:
            pthread_mutex_lock(&survivor_list_mutex);
            printf("---- Survivor Listesi ----\n");
            for (int i = 0; i < survivor_count; ++i) {
                printf("Survivor %d: (%d, %d) - %s\n", i, survivor_list[i].x, survivor_list[i].y,
                       survivor_list[i].helped ? "YARDIM EDİLDİ" : "BEKLİYOR");
            }
            printf("-------------------------\n");
            pthread_mutex_unlock(&survivor_list_mutex);

            json_object_put(jobj);
        } else {
            printf("Drone'dan mesaj (JSON değil): %s\n", buffer);
        }
    }
    printf("Drone bağlantısı kapandı (fd: %d)\n", client_fd);
    close(client_fd);
    return NULL;
}
void assign_missions() {
    pthread_mutex_lock(&survivor_list_mutex);
    pthread_mutex_lock(&drone_list_mutex);

    for (int s = 0; s < survivor_count; ++s) {
        if (survivor_list[s].helped) continue; // Zaten yardım edilmiş

        int closest = -1;
        int min_dist = 1000000;
        for (int d = 0; d < drone_count; ++d) {
            if (strcmp(drone_list[d].status, "idle") != 0) continue;
            int dx = drone_list[d].x - survivor_list[s].x;
            int dy = drone_list[d].y - survivor_list[s].y;
            int dist = dx*dx + dy*dy;
            if (dist < min_dist) {
                min_dist = dist;
                closest = d;
            }
        }
        if (closest != -1) {
            // Görev ata
            struct json_object *mission = json_object_new_object();
            json_object_object_add(mission, "type", json_object_new_string("mission"));
            struct json_object *target = json_object_new_array();
            json_object_array_add(target, json_object_new_int(survivor_list[s].x));
            json_object_array_add(target, json_object_new_int(survivor_list[s].y));
            json_object_object_add(mission, "target", target);

            const char *mission_str = json_object_to_json_string(mission);
            send(drone_list[closest].fd, mission_str, strlen(mission_str), 0);
            printf("Drone %s için görev atandı: Survivor (%d,%d)\n",
                   drone_list[closest].id, survivor_list[s].x, survivor_list[s].y);
            strncpy(drone_list[closest].status, "on_mission", sizeof(drone_list[closest].status));
            survivor_list[s].helped = 1; // Atandı, başka drone’a atanmasın
            json_object_put(mission);
        }
    }

    pthread_mutex_unlock(&drone_list_mutex);
    pthread_mutex_unlock(&survivor_list_mutex);
}
int main() {
    printf("main başladı\n");
    view_init();
    //    pthread_t view_tid;
    // pthread_create(&view_tid, NULL, view_thread_func, NULL);
    // pthread_detach(view_tid);
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }
    survivor_list[0].x = 10;
    survivor_list[0].y = 10;
    survivor_list[0].helped = 0;
    survivor_count = 1;

    strcpy(drone_list[0].id, "testdrone");
    drone_list[0].x = 20;
    drone_list[0].y = 20;
    strcpy(drone_list[0].status, "idle");
    drone_count = 1;

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    // Sadece bir kez başlat!
    pthread_t survivor_tid;
    pthread_create(&survivor_tid, NULL, survivor_generator, NULL);
    pthread_detach(survivor_tid);

    printf("Server dinleniyor (port %d)...\n", SERVER_PORT);
    fd_set readfds;
    struct timeval tv;

 while (1) {
       printf("ana döngüdeyim\n");
        pthread_mutex_lock(&survivor_list_mutex);
        pthread_mutex_lock(&drone_list_mutex);
        view_update(survivor_list, survivor_count, drone_list, drone_count);
        pthread_mutex_unlock(&drone_list_mutex);
        pthread_mutex_unlock(&survivor_list_mutex);

        // 2. Client bağlantısı kontrolü (non-blocking accept)
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms

        int activity = select(server_fd + 1, &readfds, NULL, NULL, &tv);
        if (activity > 0 && FD_ISSET(server_fd, &readfds)) {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int* client_fd = malloc(sizeof(int));
    *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
    if (*client_fd < 0) {
        perror("accept");
        free(client_fd);
        continue;
    }
    pthread_t tid;
    pthread_create(&tid, NULL, handle_drone, client_fd);
    pthread_detach(tid);
}
        // Döngü devam eder, her 100ms'de bir render ve accept kontrolü yapılır
    }

    close(server_fd);
    view_quit();
    return 0;
}
//     while (1) {
//         struct sockaddr_in client_addr;
//         socklen_t addrlen = sizeof(client_addr);
//         int* client_fd = malloc(sizeof(int));
//         *client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addrlen);
//         if (*client_fd < 0) {
//             perror("accept");
//             free(client_fd);
//             continue;
//         }
//         pthread_t tid;
//         pthread_create(&tid, NULL, handle_drone, client_fd);
//         pthread_detach(tid);
//     }

//     close(server_fd);
//     view_quit();
//     return 0;
// }