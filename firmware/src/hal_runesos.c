#include "hal_runesos.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/* ============================================================
 * HAL Mock — Simulador PC (lv_port_pc_vscode)
 *
 * Simula todos os periféricos com valores plausíveis.
 * O SD card aponta para uma pasta local "sdcard/" no workspace.
 * ============================================================ */

#define MOCK_MOUNT_POINT  "sdcard/"
#define MOCK_VOLUME       85

static uint8_t mock_volume = 60;
static uint8_t mock_backlight = 80;
static bool mock_buzzer_active = false;

/* ---- Tempo (usa clock do PC) ---- */
static void mock_get_time(runesos_hal_time_t *t)
{
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    t->year    = lt->tm_year + 1900;
    t->month   = lt->tm_mon + 1;
    t->day     = lt->tm_mday;
    t->hour    = lt->tm_hour;
    t->minute  = lt->tm_min;
    t->second  = lt->tm_sec;
    t->weekday = lt->tm_wday;
}

static void mock_set_time(const runesos_hal_time_t *t)
{
    (void)t;
    printf("[HAL MOCK] set_time chamado (no-op no simulador)\n");
}

/* ---- Bateria ---- */
static void mock_get_battery(runesos_hal_battery_t *b)
{
    b->percent   = MOCK_VOLUME;
    b->charging  = true;
    b->voltage_mv = 3850;
}

static uint8_t mock_get_battery_percent(void)
{
    return MOCK_VOLUME;
}

/* ---- Rádio ---- */
static void mock_get_radio(runesos_hal_radio_t *r)
{
    r->wifi_connected = true;
    r->wifi_rssi      = -52;
    r->wifi_signal    = runesos_rssi_to_bars(r->wifi_rssi);
    r->bt_enabled     = true;
    r->bt_connected   = true;
    strncpy(r->bt_device_name, "Teclado Dobravel BT",
            sizeof(r->bt_device_name) - 1);
    r->bt_device_name[sizeof(r->bt_device_name) - 1] = '\0';
}

/* ---- IMU (simula inclinação estática) ---- */
static void mock_get_imu(runesos_hal_imu_t *imu)
{
    imu->accel_x = 0;
    imu->accel_y = 0;
    imu->accel_z = 981;   /* 1g em mg = 981 */
    imu->gyro_x  = 0;
    imu->gyro_y  = 0;
    imu->gyro_z  = 0;
    imu->temp_c  = 25.0f;
}

/* ---- SD Card ---- */
static void mock_get_sd(runesos_hal_sd_t *sd)
{
    sd->mounted = true;
    sd->total_bytes = 16ULL * 1024 * 1024 * 1024;  /* 16 GB */
    sd->free_bytes  = 12ULL * 1024 * 1024 * 1024;  /* 12 GB livres */
    strncpy(sd->mount_point, MOCK_MOUNT_POINT,
            sizeof(sd->mount_point) - 1);
    sd->mount_point[sizeof(sd->mount_point) - 1] = '\0';
}

/* ---- Buzzer ---- */
static void mock_buzzer_on_device(uint16_t freq_hz, uint16_t duration_ms)
{
    mock_buzzer_active = true;
    printf("[HAL MOCK] Buzzer ON: %d Hz por %d ms\n",
           freq_hz, duration_ms);
}

static void mock_buzzer_off(void)
{
    mock_buzzer_active = false;
    printf("[HAL MOCK] Buzzer OFF\n");
}

static void mock_buzzer_beep(uint16_t freq_hz, uint16_t duration_ms,
                              runesos_hal_buzz_done_cb_t cb)
{
    printf("[HAL MOCK] BEEP: %d Hz / %d ms\n", freq_hz, duration_ms);
    if (cb) cb();
}

/* ---- Volume ---- */
static void mock_set_volume(uint8_t percent)
{
    mock_volume = percent > 100 ? 100 : percent;
}

static uint8_t mock_get_volume(void)
{
    return mock_volume;
}

/* ---- Backlight ---- */
static void mock_set_backlight(uint8_t percent)
{
    mock_backlight = percent > 100 ? 100 : percent;
}

static uint8_t mock_get_backlight(void)
{
    return mock_backlight;
}

/* ---- Power ---- */
static void mock_enter_sleep(void)
{
    printf("[HAL MOCK] enter_sleep (no-op)\n");
}

static void mock_wake_up(void)
{
    printf("[HAL MOCK] wake_up (no-op)\n");
}

/* ============================================================
 * Instância exportada — usada pelo main.c do simulador
 * ============================================================ */
const runesos_hal_t runesos_hal_mock = {
    .get_time           = mock_get_time,
    .set_time           = mock_set_time,
    .get_battery        = mock_get_battery,
    .get_battery_percent = mock_get_battery_percent,
    .get_radio          = mock_get_radio,
    .get_imu            = mock_get_imu,
    .get_sd             = mock_get_sd,
    .buzzer_on          = mock_buzzer_on_device,
    .buzzer_off         = mock_buzzer_off,
    .buzzer_beep        = mock_buzzer_beep,
    .set_volume         = mock_set_volume,
    .get_volume         = mock_get_volume,
    .set_backlight      = mock_set_backlight,
    .get_backlight      = mock_get_backlight,
    .enter_sleep        = mock_enter_sleep,
    .wake_up            = mock_wake_up,
};