#ifndef VIEW_H
#define VIEW_H
#include "common.h"

void view_init();
void view_update(Survivor* survivors, int survivor_count, DroneInfo* drones, int drone_count);
void view_quit();

#endif