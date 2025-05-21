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
    (void)arg;
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
        void send_heartbeat_to_all() {
    pthread_mutex_lock(&drone_list_mutex);
    for (int i = 0; i < drone_count; ++i) {
        struct json_object *hb = json_object_new_object();
        json_object_object_add(hb, "type", json_object_new_string("HEARTBEAT"));
        json_object_object_add(hb, "timestamp", json_object_new_int(time(NULL)));
        const char *hb_str = json_object_to_json_string(hb);
        send(drone_list[i].fd, hb_str, strlen(hb_str), 0);
        json_object_put(hb);
    }
    pthread_mutex_unlock(&drone_list_mutex);
}
void* handle_drone(void* arg) {
    printf("Yeni drone thread başlatıldı!\n");
    int client_fd = *(int*)arg;
    free(arg);
    char buffer[1024];

    // --- 1. HANDSHAKE mesajını bekle ---
    ssize_t n = recv(client_fd, buffer, sizeof(buffer)-1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        struct json_object *jobj = json_tokener_parse(buffer);
        if (jobj) {
            const char *type = json_object_get_string(json_object_object_get(jobj, "type"));
            if (type && strcmp(type, "HANDSHAKE") == 0) {
                // HANDSHAKE_ACK mesajı oluştur ve gönder
                struct json_object *ack = json_object_new_object();
                json_object_object_add(ack, "type", json_object_new_string("HANDSHAKE_ACK"));
                json_object_object_add(ack, "session_id", json_object_new_string("S123")); // örnek session id
                struct json_object *cfg = json_object_new_object();
                json_object_object_add(cfg, "status_update_interval", json_object_new_int(5));
                json_object_object_add(cfg, "heartbeat_interval", json_object_new_int(10));
                json_object_object_add(ack, "config", cfg);

                const char *ack_msg = json_object_to_json_string(ack);
                send(client_fd, ack_msg, strlen(ack_msg), 0);
                json_object_put(ack);

                printf("Drone handshake alındı ve ACK gönderildi.\n");
            }
            json_object_put(jobj);
        }
    } else {
        close(client_fd);
        return NULL;
    }

    // --- 2. Sürekli mesaj dinleme döngüsü ---
    while (1) {
        ssize_t n = recv(client_fd, buffer, sizeof(buffer)-1, 0);
        if (n <= 0) break; // bağlantı koptu
        buffer[n] = '\0';
        printf("Gelen mesaj: %s\n", buffer);

        struct json_object *jobj = json_tokener_parse(buffer);
        if (jobj) {
            const char *type = json_object_get_string(json_object_object_get(jobj, "type"));
            if (type) {
                if (strcmp(type, "STATUS_UPDATE") == 0) {
                    // --- STATUS_UPDATE işleme ---
                    const char *drone_id = json_object_get_string(json_object_object_get(jobj, "drone_id"));
                    struct json_object *loc = json_object_object_get(jobj, "location");
                    int x = json_object_get_int(json_object_object_get(loc, "x"));
                    int y = json_object_get_int(json_object_object_get(loc, "y"));
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
                        drone_list[drone_count].has_target = 0;
                        drone_count++;
                    }
                    pthread_mutex_unlock(&drone_list_mutex);
                }
                    else if (strcmp(type, "MISSION_COMPLETE") == 0) {
    const char *mission_id = json_object_get_string(json_object_object_get(jobj, "mission_id"));
    // mission_id ile survivor'u bul:
    for (int i = 0; i < survivor_count; ++i) {
        char expected_id[32];
        snprintf(expected_id, sizeof(expected_id), "M%03d", i);
        if (strcmp(mission_id, expected_id) == 0) {
            survivor_list[i].helped = 1;
            break;
        }
    }
    pthread_mutex_unlock(&drone_list_mutex);

                    // Konsola yazdır
                    printf("---- Drone Listesi ----\n");
                    for (int i = 0; i < drone_count; ++i) {
                        printf("ID: %s | Konum: (%d, %d) | Durum: %s\n",
                            drone_list[i].id, drone_list[i].x, drone_list[i].y, drone_list[i].status);
                    }
                    printf("-----------------------\n");

                    pthread_mutex_lock(&survivor_list_mutex);
                    printf("---- Survivor Listesi ----\n");
                    for (int i = 0; i < survivor_count; ++i) {
                        printf("Survivor %d: (%d, %d) - %s\n", i, survivor_list[i].x, survivor_list[i].y,
                            survivor_list[i].helped ? "YARDIM EDİLDİ" : "BEKLİYOR");
                    }
                    printf("-------------------------\n");
                    pthread_mutex_unlock(&survivor_list_mutex);

                }
                // Buraya diğer mesaj tipleri (MISSION_COMPLETE, HEARTBEAT_RESPONSE, vs.) eklenebilir
            }
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
        // assign_missions içinde:
if (closest != -1) {
    struct json_object *mission = json_object_new_object();
    char mission_id[32];
    snprintf(mission_id, sizeof(mission_id), "M%03d", s);
    json_object_object_add(mission, "type", json_object_new_string("ASSIGN_MISSION"));
    json_object_object_add(mission, "mission_id", json_object_new_string(mission_id));
  
            json_object_object_add(mission, "priority", json_object_new_string("high")); // örnek öncelik

            struct json_object *target = json_object_new_object();
            json_object_object_add(target, "x", json_object_new_int(survivor_list[s].x));
            json_object_object_add(target, "y", json_object_new_int(survivor_list[s].y));
            json_object_object_add(mission, "target", target);

            json_object_object_add(mission, "expiry", json_object_new_int(time(NULL) + 600)); // 10 dakika sonrası
            json_object_object_add(mission, "checksum", json_object_new_string("a1b2c3")); // örnek checksum

            const char *mission_str = json_object_to_json_string(mission);
            send(drone_list[closest].fd, mission_str, strlen(mission_str), 0);
            printf("Drone %s için görev atandı: Survivor (%d,%d)\n",
                   drone_list[closest].id, survivor_list[s].x, survivor_list[s].y);
            strncpy(drone_list[closest].status, "on_mission", sizeof(drone_list[closest].status));
            survivor_list[s].assigned = 1;
            // survivor_list[s].helped = 1;

            drone_list[closest].target_x = survivor_list[s].x;
            drone_list[closest].target_y = survivor_list[s].y;
            drone_list[closest].has_target = 1;

            json_object_put(mission);
        }
    }

    pthread_mutex_unlock(&drone_list_mutex);
    pthread_mutex_unlock(&survivor_list_mutex);
}
int main() {
    printf("main başladı\n");
    view_init();

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

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(1);
    }

    // Survivor üretici thread'i başlat
    pthread_t survivor_tid;
    pthread_create(&survivor_tid, NULL, survivor_generator, NULL);
    pthread_detach(survivor_tid);

    printf("Server dinleniyor (port %d)...\n", SERVER_PORT);

    fd_set readfds;
    struct timeval tv;

    while (1) {
        // Görsel güncelleme
        pthread_mutex_lock(&survivor_list_mutex);
        pthread_mutex_lock(&drone_list_mutex);
        view_update(survivor_list, survivor_count, drone_list, drone_count);
        pthread_mutex_unlock(&drone_list_mutex);
        pthread_mutex_unlock(&survivor_list_mutex);

        // Yeni client bağlantısı kontrolü (non-blocking accept)
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms
        assign_missions();

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
        // Her 10 döngüde bir heartbeat gönder (örnek):
static int heartbeat_counter = 0;
heartbeat_counter++;
if (heartbeat_counter >= 100) { // 100 x 100ms = 10 saniye
    send_heartbeat_to_all();
    heartbeat_counter = 0;
}
    }

    close(server_fd);
    view_quit();
    return 0;
}
