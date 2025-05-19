#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "view.h"
#include "common.h"
#include <unistd.h>
#define WIN_W 800
#define WIN_H 800

SDL_Window* win = NULL;
SDL_Renderer* ren = NULL;
TTF_Font* font = NULL;

void view_init() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 16);
    win = SDL_CreateWindow("Emergency Drone Coordination", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W, WIN_H, 0);
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
}



extern SDL_Renderer* ren;
extern TTF_Font* font;

// Yardımcı: Ekrana metin yaz
static void draw_text(int x, int y, const char *txt, SDL_Color color) {
    if (!font) return;
    SDL_Surface *surface = TTF_RenderText_Solid(font, txt, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_Rect dst = {x, y, surface->w, surface->h};
    SDL_RenderCopy(ren, texture, NULL, &dst);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Daire çiz (drone için)
static void draw_circle(int x, int y, int r, SDL_Color color) {
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    for (int w = 0; w < r * 2; w++) {
        for (int h = 0; h < r * 2; h++) {
            int dx = r - w;
            int dy = r - h;
            if ((dx*dx + dy*dy) <= (r * r))
                SDL_RenderDrawPoint(ren, x + dx, y + dy);
        }
    }
}

// view_update fonksiyonu
void view_update(Survivor* survivors, int survivor_count, DroneInfo* drones, int drone_count) {
    printf("view_update çağrıldı: drone_count=%d\n", drone_count);
    for (int i = 0; i < drone_count; ++i) {
        printf("Çizilecek drone: %s (%d,%d)\n", drones[i].id, drones[i].x, drones[i].y);
    }
    SDL_SetRenderDrawColor(ren, 240, 240, 240, 255); // Açık gri arka plan
    SDL_RenderClear(ren);

    // Survivorları çiz
    for (int i = 0; i < survivor_count; ++i) {
        SDL_Color color;
        if (survivors[i].helped)
            color = (SDL_Color){128, 128, 128, 255}; // Gri
        else
            color = (SDL_Color){220, 0, 0, 255};     // Kırmızı
        SDL_Rect rect = { survivors[i].x * 8, survivors[i].y * 8, 16, 16 };
        SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(ren, &rect);

        // Survivor etiketi
        char label[16];
        snprintf(label, sizeof(label), "S%d", i+1);
        draw_text(rect.x + 2, rect.y - 18, label, color);
    }

    // Droneları çiz ve görevdeyse yeşil çizgi çiz
    for (int i = 0; i < drone_count; ++i) {
        SDL_Color blue = {0, 100, 255, 255};
        draw_circle(drones[i].x * 8 + 8, drones[i].y * 8 + 8, 10, blue);

        // Drone etiketi
        char label[16];
        snprintf(label, sizeof(label), "%s", drones[i].id);
        draw_text(drones[i].x * 8, drones[i].y * 8 + 18, label, blue);

        // Görevdeyse survivor'a yeşil çizgi
        if (drones[i].has_target) {
            SDL_SetRenderDrawColor(ren, 0, 200, 0, 255);
            SDL_RenderDrawLine(ren,
                drones[i].x * 8 + 8, drones[i].y * 8 + 8,
                drones[i].target_x * 8 + 8, drones[i].target_y * 8 + 8);
        }
    }

    // İstatistik paneli ve legend
    SDL_Color black = {0,0,0,255};
    SDL_Color blue = {0,100,255,255};
    SDL_Color red = {220,0,0,255};
    SDL_Color green = {0,200,0,255};
    SDL_Color gray = {128,128,128,255};

    char stats[128];
    snprintf(stats, sizeof(stats), "Drones: %d   Survivors: %d", drone_count, survivor_count);
    draw_text(20, 20, stats, black);

    draw_text(20, 50, "Legend:", black);
    draw_text(40, 70, "Blue Circle: Drone", blue);
    draw_text(40, 90, "Red Square: Survivor (waiting)", red);
    draw_text(40, 110, "Gray Square: Survivor (rescued)", gray);
    draw_text(40, 130, "Green Line: Active Mission", green);

    SDL_RenderPresent(ren);
}