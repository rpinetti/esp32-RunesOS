#include "radio_service.h"
#include <stddef.h>

const runesos_hal_radio_t *runesos_hal_radio = NULL;

void runesos_radio_get(runesos_radio_state_t *s)
{
    if (runesos_hal_radio && runesos_hal_radio->get_state) {
        runesos_hal_radio->get_state(s);
    } else {
        s->wifi_connected = false;
        s->wifi_signal = 0;
        s->bt_connected = false;
    }
}