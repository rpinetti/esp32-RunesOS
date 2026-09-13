#include "radio_service.h"
#include "runesos_fw.h"

void runesos_radio_get(runesos_hal_radio_t *s)
{
    const runesos_hal_t *hal = runesos_fw_get_hal();
    if (hal && hal->get_radio) {
        hal->get_radio(s);
    } else {
        s->wifi_connected = false;
        s->wifi_rssi      = -100;
        s->wifi_signal    = 0;
        s->bt_enabled     = false;
        s->bt_connected   = false;
        s->bt_device_name[0] = '\0';
    }
}