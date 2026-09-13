#include "time_service.h"
#include "runesos_fw.h"

void runesos_time_get(runesos_hal_time_t *t)
{
    const runesos_hal_t *hal = runesos_fw_get_hal();
    if (hal && hal->get_time) {
        hal->get_time(t);
    } else {
        t->year    = 2000;
        t->month   = 1;
        t->day     = 1;
        t->hour    = 0;
        t->minute  = 0;
        t->second  = 0;
        t->weekday = 0;
    }
}