#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Kullanım: %s DRONE_ID\n", argv[0]);
        return 1;
    }
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    printf("Server'a bağlanıldı!\n");
    struct json_object *handshake = json_object_new_object();
json_object_object_add(handshake, "type", json_object_new_string("HANDSHAKE"));
json_object_object_add(handshake, "drone_id", json_object_new_string(argv[1]));
struct json_object *cap = json_object_new_object();
json_object_object_add(cap, "max_speed", json_object_new_int(30));
json_object_object_add(cap, "battery_capacity", json_object_new_int(100));
json_object_object_add(cap, "payload", json_object_new_string("medical"));
json_object_object_add(handshake, "capabilities", cap);

const char *msg = json_object_to_json_string(handshake);
send(sockfd, msg, strlen(msg), 0);
json_object_put(handshake);

    int x = 10, y = 20;
int target_x = -1, target_y = -1;
char status[16] = "idle";
char current_mission_id[32] = "";

while (1) {
    // 1. STATUS_UPDATE mesajı gönder
    struct json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "type", json_object_new_string("STATUS_UPDATE"));
    json_object_object_add(jobj, "drone_id", json_object_new_string(argv[1]));
    json_object_object_add(jobj, "timestamp", json_object_new_int(time(NULL)));
    struct json_object *loc = json_object_new_object();
    json_object_object_add(loc, "x", json_object_new_int(x));
    json_object_object_add(loc, "y", json_object_new_int(y));
    json_object_object_add(jobj, "location", loc);
    json_object_object_add(jobj, "status", json_object_new_string(status));
    json_object_object_add(jobj, "battery", json_object_new_int(100)); // örnek
    json_object_object_add(jobj, "speed", json_object_new_int(5));     // örnek

    const char *json_str = json_object_to_json_string(jobj);
    if (send(sockfd, json_str, strlen(json_str), 0) < 0) {
        perror("send");
        json_object_put(jobj);
        break;
    }
    json_object_put(jobj);

    // 2. Server'dan mesaj bekle
    char buffer[512];
    ssize_t n = recv(sockfd, buffer, sizeof(buffer)-1, MSG_DONTWAIT);
    if (n > 0) {
        buffer[n] = '\0';
        struct json_object *reply = json_tokener_parse(buffer);
        if (reply) {
            const char *type = json_object_get_string(json_object_object_get(reply, "type"));
            if (type && strcmp(type, "ASSIGN_MISSION") == 0) {
    // Görev bilgilerini al
    const char *mission_id = json_object_get_string(json_object_object_get(reply, "mission_id"));
    struct json_object *target = json_object_object_get(reply, "target");
    int tx = json_object_get_int(json_object_object_get(target, "x"));
    int ty = json_object_get_int(json_object_object_get(target, "y"));
    printf("Yeni görev atandı! Mission ID: %s, Hedef: (%d,%d)\n", mission_id, tx, ty);

    target_x = tx;
    target_y = ty;
    strncpy(current_mission_id, mission_id, sizeof(current_mission_id));
    strcpy(status, "on_mission");
}
            else if (type && strcmp(type, "HEARTBEAT") == 0) {
            struct json_object *resp = json_object_new_object();
            json_object_object_add(resp, "type", json_object_new_string("HEARTBEAT_RESPONSE"));
            json_object_object_add(resp, "drone_id", json_object_new_string(argv[1]));
            json_object_object_add(resp, "timestamp", json_object_new_int(time(NULL)));
            const char *resp_str = json_object_to_json_string(resp);
            send(sockfd, resp_str, strlen(resp_str), 0);
            json_object_put(resp);
        }
            // Diğer mesaj tipleri burada işlenebilir (HEARTBEAT, ERROR, ...)
            json_object_put(reply);
        }
    }
    

   // 3. Eğer görevdeyse hedefe ilerle
    if (strcmp(status, "on_mission") == 0 && target_x >= 0 && target_y >= 0) {
    if (x < target_x) x++;
    else if (x > target_x) x--;
    if (y < target_y) y++;
    else if (y > target_y) y--;

    // Hedefe ulaştıysa
    if (x == target_x && y == target_y) {
        printf("Görev tamamlandı! Mission ID: %s\n", current_mission_id);
        strcpy(status, "idle");
        target_x = target_y = -1;

        // MISSION_COMPLETE mesajı gönder
        struct json_object *complete = json_object_new_object();
        json_object_object_add(complete, "type", json_object_new_string("MISSION_COMPLETE"));
        json_object_object_add(complete, "drone_id", json_object_new_string(argv[1]));
        json_object_object_add(complete, "mission_id", json_object_new_string(current_mission_id));
        json_object_object_add(complete, "timestamp", json_object_new_int(time(NULL)));
        json_object_object_add(complete, "success", json_object_new_boolean(1));
        json_object_object_add(complete, "details", json_object_new_string("Delivered aid to survivor."));
        const char *comp_str = json_object_to_json_string(complete);
        send(sockfd, comp_str, strlen(comp_str), 0);
        json_object_put(complete);

        current_mission_id[0] = '\0';
    }
}
    printf("Drone konumu: (%d, %d), status: %s\n", x, y, status);
    sleep(1);
}

    close(sockfd);
    return 0;
}


// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <json-c/json.h>

// #define SERVER_IP "127.0.0.1"
// #define SERVER_PORT 12345

// int main(int argc, char *argv[]) {
//     if (argc < 2) {
//         printf("Kullanım: %s DRONE_ID\n", argv[0]);
//         return 1;
//     }
//     int sockfd = socket(AF_INET, SOCK_STREAM, 0);
//     if (sockfd < 0) {
//         perror("socket");
//         exit(1);
//     }

//     struct sockaddr_in serv_addr;
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(SERVER_PORT);
//     if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
//         perror("inet_pton");
//         close(sockfd);
//         exit(1);
//     }

//     if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
//         perror("connect");
//         close(sockfd);
//         exit(1);
//     }

//     printf("Server'a bağlanıldı!\n");

//     int x = 10, y = 20;
//     for (int i = 0; i < 10; ++i) { // 10 kez durum gönder, istersen sonsuz döngüye alabilirsin
//         // JSON ile durum mesajı oluştur
//         struct json_object *jobj = json_object_new_object();
//     json_object_object_add(jobj, "drone_id", json_object_new_string(argv[1]));
//         json_object_object_add(jobj, "status", json_object_new_string("idle"));
//         struct json_object *loc = json_object_new_array();
//         json_object_array_add(loc, json_object_new_int(x));
//         json_object_array_add(loc, json_object_new_int(y));
//         json_object_object_add(jobj, "location", loc);

//         const char *json_str = json_object_to_json_string(jobj);
//         if (send(sockfd, json_str, strlen(json_str), 0) < 0) {
//             perror("send");
//             json_object_put(jobj);
//             break;
//         }

//         printf("JSON mesajı gönderildi: %s\n", json_str);
//         json_object_put(jobj);
        
//         // Server'dan mesaj bekle (ör: görev ataması)
//         char buffer[256];
//         ssize_t n = recv(sockfd, buffer, sizeof(buffer)-1, MSG_DONTWAIT);
//         if (n > 0) {
//             buffer[n] = '\0';
//             printf("Server'dan gelen mesaj: %s\n", buffer);
//         }

//         sleep(1);
//         x++; y++; // Her seferinde konumu değiştir
//     }

//     close(sockfd);
//     return 0;
// }