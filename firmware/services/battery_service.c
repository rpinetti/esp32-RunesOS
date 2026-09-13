#include "battery_service.h"
#include "runesos_fw.h"

void runesos_battery_get(runesos_hal_battery_t *s)
{
    const runesos_hal_t *hal = runesos_fw_get_hal();
    if (hal && hal->get_battery) {
        hal->get_battery(s);
    } else {
        s->percent    = 0;
        s->charging   = false;
        s->voltage_mv = 0;
    }
}