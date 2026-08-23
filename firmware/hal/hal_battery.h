#ifndef RUNESOS_HAL_BATTERY_H
#define RUNESOS_HAL_BATTERY_H

#include <stdbool.h>

typedef struct {
    int  level;      /* 0-100 */
    bool charging;   /* true se carregando */
} runesos_battery_state_t;

typedef struct {
    void (*get_state)(runesos_battery_state_t *s);
} runesos_hal_battery_t;

extern const runesos_hal_battery_t *runesos_hal_battery;

#endif