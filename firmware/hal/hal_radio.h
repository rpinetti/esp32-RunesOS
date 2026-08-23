#ifndef RUNESOS_HAL_RADIO_H
#define RUNESOS_HAL_RADIO_H

#include <stdbool.h>

typedef struct {
    bool wifi_connected;
    int  wifi_signal;    /* 0-100 (RSSI mapeado) */
    bool bt_connected;
} runesos_radio_state_t;

typedef struct {
    void (*get_state)(runesos_radio_state_t *s);
} runesos_hal_radio_t;

extern const runesos_hal_radio_t *runesos_hal_radio;

#endif