#ifndef RUNESOS_QUICKSETTINGS_H
#define RUNESOS_QUICKSETTINGS_H

#include "lvgl.h"
#include "lvgl_compat.h"
#include "hal_runesos.h"


void runesos_quicksettings_toggle(lv_obj_t *parent, const runesos_hal_t *hal);
bool runesos_quicksettings_is_open(void);
void runesos_quicksettings_close(void);

#endif // RUNESOS_QUICKSETTINGS_H

