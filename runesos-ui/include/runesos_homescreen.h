#ifndef RUNESOS_HOMESCREEN_H
#define RUNESOS_HOMESCREEN_H

#include "lvgl.h"
#include "hal_runesos.h"

void runesos_homescreen_create(lv_obj_t *root,
                               const runesos_hal_t *hal,
                               void (*on_app_launch)(uint8_t index));

#endif
