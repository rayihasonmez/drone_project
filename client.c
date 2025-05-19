#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <json-c/json.h>

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

    int x = 10, y = 20;
    for (int i = 0; i < 10; ++i) { // 10 kez durum gönder, istersen sonsuz döngüye alabilirsin
        // JSON ile durum mesajı oluştur
        struct json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "drone_id", json_object_new_string(argv[1]));
        json_object_object_add(jobj, "status", json_object_new_string("idle"));
        struct json_object *loc = json_object_new_array();
        json_object_array_add(loc, json_object_new_int(x));
        json_object_array_add(loc, json_object_new_int(y));
        json_object_object_add(jobj, "location", loc);

        const char *json_str = json_object_to_json_string(jobj);
        if (send(sockfd, json_str, strlen(json_str), 0) < 0) {
            perror("send");
            json_object_put(jobj);
            break;
        }

        printf("JSON mesajı gönderildi: %s\n", json_str);
        json_object_put(jobj);
        
        // Server'dan mesaj bekle (ör: görev ataması)
        char buffer[256];
        ssize_t n = recv(sockfd, buffer, sizeof(buffer)-1, MSG_DONTWAIT);
        if (n > 0) {
            buffer[n] = '\0';
            printf("Server'dan gelen mesaj: %s\n", buffer);
        }

        sleep(1);
        x++; y++; // Her seferinde konumu değiştir
    }

    close(sockfd);
    return 0;
}