#include "battery_service.h"
#include <stddef.h>

const runesos_hal_battery_t *runesos_hal_battery = NULL;

void runesos_battery_get(runesos_battery_state_t *s)
{
    if (runesos_hal_battery && runesos_hal_battery->get_state) {
        runesos_hal_battery->get_state(s);
    } else {
        s->level = 0;
        s->charging = false;
    }
}