#ifndef COMMON_H
#define COMMON_H

typedef struct {
    int x, y;
    int helped;      // 0: bekliyor, 1: yardım edildi
    int assigned;    // 0: atanmadı, 1: bir drone atandı (görselde yeşil çizgi için)
} Survivor;

#define DRONE_ID_LEN 32
typedef struct {
    char id[DRONE_ID_LEN];
    int x, y;
    char status[16];
    int fd;
    int target_x, target_y; // Hedef survivor'ın koordinatları
    int has_target;         // 1 ise hedef var, 0 ise yok
} DroneInfo;

#endif