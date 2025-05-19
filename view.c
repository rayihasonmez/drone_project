#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "view.h"
#include "common.h"

#define WIN_W 800
#define WIN_H 800

static SDL_Window* win = NULL;
static SDL_Renderer* ren = NULL;

void view_init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
    }
    if (TTF_Init() < 0) {
        printf("TTF_Init error: %s\n", TTF_GetError());
    }
    win = SDL_CreateWindow("Emergency Drone Coordination", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, 0);
    if (!win) printf("SDL_CreateWindow error: %s\n", SDL_GetError());
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (!ren) printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
}

void view_update(Survivor* survivors, int survivor_count, DroneInfo* drones, int drone_count) {
    printf("view_update: %d survivor, %d drone\n", survivor_count, drone_count);
    SDL_SetRenderDrawColor(ren, 255, 255, 255, 255); // Beyaz arka plan
    SDL_RenderClear(ren);

    // Survivor'lar (kırmızı)
    for (int i = 0; i < survivor_count; ++i) {
        SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
        SDL_Rect rect = { survivors[i].x * 8, survivors[i].y * 8, 10, 10 };
        SDL_RenderFillRect(ren, &rect);
    }

    // Drone'lar (mavi/yeşil)
    for (int i = 0; i < drone_count; ++i) {
        if (strcmp(drones[i].status, "idle") == 0)
            SDL_SetRenderDrawColor(ren, 0, 0, 255, 255); // Mavi
        else
            SDL_SetRenderDrawColor(ren, 0, 255, 0, 255); // Yeşil
        SDL_Rect rect = { drones[i].x * 8, drones[i].y * 8, 10, 10 };
        SDL_RenderFillRect(ren, &rect);
    }

    SDL_RenderPresent(ren);
}

// void view_update(Survivor* survivors, int survivor_count, DroneInfo* drones, int drone_count) {
//     SDL_SetRenderDrawColor(ren, 255, 255, 255, 255); // Beyaz arka plan
//     SDL_RenderClear(ren);

//     // Survivor'lar (kırmızı)
//     for (int i = 0; i < survivor_count; ++i) {
//         SDL_SetRenderDrawColor(ren, 255, 0, 0, 255);
//         SDL_Rect rect = { survivors[i].x * 8, survivors[i].y * 8, 10, 10 };
//         SDL_RenderFillRect(ren, &rect);
//     }

//     // Drone'lar (mavi/yeşil)
//     for (int i = 0; i < drone_count; ++i) {
//         if (strcmp(drones[i].status, "idle") == 0)
//             SDL_SetRenderDrawColor(ren, 0, 0, 255, 255); // Mavi
//         else
//             SDL_SetRenderDrawColor(ren, 0, 255, 0, 255); // Yeşil
//         SDL_Rect rect = { drones[i].x * 8, drones[i].y * 8, 10, 10 };
//         SDL_RenderFillRect(ren, &rect);
//     }
// if (!win) printf("SDL_CreateWindow error: %s\n", SDL_GetError());
// if (!ren) printf("SDL_CreateRenderer error: %s\n", SDL_GetError());
//     SDL_RenderPresent(ren);
// }

// void view_quit() {
//     SDL_DestroyRenderer(ren);
//     SDL_DestroyWindow(win);
//     TTF_Quit();
//     SDL_Quit();
// }



// #include "view.h"
// #include "drone.h"
// #include "survivor.h"
// #include <SDL2/SDL.h>
// #include <SDL2/SDL_ttf.h>
// #include <stdio.h>
// #include <string.h>
// #include <time.h>
// #include "list.h"

// static SDL_Window *win = NULL;
// static SDL_Renderer *ren = NULL;
// static TTF_Font *font = NULL;
// static char last_message[64] = "";
// static time_t last_message_time = 0;

// // Yardımcı: Ekrana metin yaz
// static void draw_text(int x, int y, const char *txt, SDL_Color color) {
//     if (!font) return;
//     SDL_Surface *surface = TTF_RenderText_Solid(font, txt, color);
//     SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surface);
//     SDL_Rect dst = {x, y, surface->w, surface->h};
//     SDL_RenderCopy(ren, texture, NULL, &dst);
//     SDL_FreeSurface(surface);
//     SDL_DestroyTexture(texture);
// }

// int view_init() {
//     if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;
//     if (TTF_Init() < 0) return -2;
//     font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16);
//     if (!font) return -3;
//     win = SDL_CreateWindow("Drone Rescue", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
//     if (!win) return -4;
//     ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
//     if (!ren) return -5;
//     return 0;
// }

// void view_quit() {
//     if (font) TTF_CloseFont(font);
//     TTF_Quit();
//     if (ren) SDL_DestroyRenderer(ren);
//     if (win) SDL_DestroyWindow(win);
//     SDL_Quit();
// }

// void view_render(List *drones, List *survivors) {
//     // Arka planı açık gri yap
//     SDL_SetRenderDrawColor(ren, 220, 220, 220, 255);
//     SDL_RenderClear(ren);

//     time_t now = time(NULL);

//     // Survivor fade-out ve silme işlemi: rescued_at > 0 && geçen süre > 2 ise sil
//     pthread_mutex_lock(&survivors->mutex);
//     ListNode *prev = NULL, *n = survivors->head;
//     while (n) {
//         Survivor *s = (Survivor*)n->data;
//         if (s->rescued_at > 0 && now - s->rescued_at >= 2) {
//             // 2 saniye geçti, survivor artık görünmeyecek
//             ListNode *to_delete = n;
//             if (prev)
//                 prev->next = n->next;
//             else
//                 survivors->head = n->next;
//             n = n->next;
//             survivor_destroy(to_delete->data);
//             free(to_delete);
//             continue;
//         }
//         prev = n;
//         n = n->next;
//     }
//     pthread_mutex_unlock(&survivors->mutex);

//     // Survivor’lar: Kırmızı (normal), Gri/Şeffaf (fade)
//     SDL_Color red = {200,0,0,255};
//     SDL_Color fade = {128,128,128,128};
//     pthread_mutex_lock(&survivors->mutex);
//     for (ListNode *n = survivors->head; n; n = n->next) {
//         Survivor *s = (Survivor*)n->data;
//         SDL_Rect r = {(int)s->x-6, (int)s->y-6, 12, 12};
//         char txt[16]; snprintf(txt, sizeof(txt), "S%d", s->id);
//         if (s->rescued_at == 0) {
//             SDL_SetRenderDrawColor(ren, 200, 0, 0, 255);
//             SDL_RenderFillRect(ren, &r);
//             draw_text((int)s->x+8, (int)s->y-2, txt, red);
//         } else {
//             // Fade: gri ve şeffaf, üzerine "RESCUED" yazısı
//             Uint8 alpha = 128 + (now-s->rescued_at)*64; // 2 saniyede 128->255 arası
//             SDL_SetRenderDrawColor(ren, 128, 128, 128, alpha);
//             SDL_RenderFillRect(ren, &r);
//             draw_text((int)s->x+8, (int)s->y-2, txt, fade);
//             draw_text((int)s->x-10, (int)s->y+14, "RESCUED", fade);

//             // Kayar bilgi mesajı: son kurtarılan survivor
//             if (now - s->rescued_at < 1 && strlen(last_message) == 0) {
//                 snprintf(last_message, sizeof(last_message), "Survivor S%d rescued!", s->id);
//                 last_message_time = now;
//             }
//         }
//     }
//     pthread_mutex_unlock(&survivors->mutex);

//     // Dronelar: Mavi daire, ID ve durum etiketi
//     SDL_Color blue = {0,0,200,255};
//     SDL_Color green = {0,128,0,255};
//     pthread_mutex_lock(&drones->mutex);
//     for (ListNode *n = drones->head; n; n = n->next) {
//         Drone *d = (Drone*)n->data;
//         SDL_SetRenderDrawColor(ren, 0, 0, 200, 255);
//         SDL_Rect r = { (int)d->x-10, (int)d->y-10, 20, 20 };
//         SDL_RenderDrawRect(ren, &r);
//         SDL_RenderFillRect(ren, &r);

//         char txt[32];
//         snprintf(txt, sizeof(txt), "D%d (%s)", d->id,
//             d->status == DRONE_IDLE ? "Idle" :
//             d->status == DRONE_ON_MISSION ? "Mission" : "Moving");
//         draw_text((int)d->x+14, (int)d->y-6, txt, blue);

//         // Görevdeyse hedefe çizgi (survivor'a)
//         if (d->status == DRONE_ON_MISSION) {
//             SDL_SetRenderDrawColor(ren, 0,200,0,255);
//             pthread_mutex_lock(&survivors->mutex);
//             for (ListNode *snode = survivors->head; snode; snode = snode->next) {
//                 Survivor *s = (Survivor*)snode->data;
//                 if (s->id == d->assigned_survivor_id) {
//                     SDL_RenderDrawLine(ren, (int)d->x, (int)d->y, (int)s->x, (int)s->y);
//                     break;
//                 }
//             }
//             pthread_mutex_unlock(&survivors->mutex);
//         }
//     }
//     pthread_mutex_unlock(&drones->mutex);

//     // Legend ve istatistikler
//     SDL_Color black = {0,0,0,255};
//     char stats[128];
//     snprintf(stats, sizeof(stats), "Drones: %zu   Survivors: %zu",
//         list_size(drones), list_size(survivors));
//     draw_text(10, 10, stats, black);
//     draw_text(10, 40, "Legend:", black);
//     draw_text(30, 60, "Blue: Drone", blue);
//     draw_text(30, 80, "Red: Survivor", red);
//     draw_text(30, 100, "Gray: Recently rescued", fade);
//     draw_text(30, 120, "Green Line: Active Mission", green);

//     // Son info mesajı
//     if (strlen(last_message) > 0 && now - last_message_time < 2) {
//         SDL_Color orange = {255,140,0,255};
//         draw_text(300, 20, last_message, orange);
//     } else if (now - last_message_time >= 2) {
//         last_message[0] = 0;
//     }

//     SDL_RenderPresent(ren);
//     SDL_Delay(33);
// }

