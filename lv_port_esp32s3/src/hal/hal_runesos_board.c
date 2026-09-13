#include "hal_runesos.h"
#include "PCF85063.h"

static void board_get_time(runesos_hal_time_t *t)
{
    datetime_t dt;
    PCF85063_Read_Time(&dt);
    t->year    = dt.year;
    t->month   = dt.month;
    t->day     = dt.day;
    t->hour    = dt.hour;
    t->minute  = dt.minute;
    t->second  = dt.second;
    t->weekday = dt.dotw;   /* 0=domingo ... 6=sábado — igual ao HAL */
}

static void board_set_time(const runesos_hal_time_t *t)
{
    datetime_t dt = { .year=t->year, .month=t->month, .day=t->day,
                      .dotw=t->weekday, .hour=t->hour,
                      .minute=t->minute, .second=t->second };
    PCF85063_Set_All(dt);
}

const runesos_hal_t runesos_hal_board = {
    .get_time = board_get_time,
    .set_time = board_set_time,
    /* demais ponteiros: a completar */
};