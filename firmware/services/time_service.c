#include "time_service.h"
#include <stddef.h>

const runesos_hal_time_t *runesos_hal_time = NULL;

void runesos_time_get(runesos_time_t *t)
{
    if (runesos_hal_time && runesos_hal_time->get_time) {
        runesos_hal_time->get_time(t);
    } else {
        t->hour = 0; t->minute = 0; t->second = 0;
    }
}