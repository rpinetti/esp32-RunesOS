#ifndef RUNESOS_HAL_TIME_H
#define RUNESOS_HAL_TIME_H

typedef struct {
    int hour;
    int minute;
    int second;
} runesos_time_t;

/* Backend agnóstico: no PC vem do sistema, na placa virá do RTC PCF85063 */
typedef struct {
    void (*get_time)(runesos_time_t *t);
} runesos_hal_time_t;

extern const runesos_hal_time_t *runesos_hal_time;

#endif