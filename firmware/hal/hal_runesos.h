#ifndef RUNESOS_HAL_H
#define RUNESOS_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/* ============================================================
 * RunesOS — Hardware Abstraction Layer
 *
 * Define interfaces abstratas para todos os periféricos da
 * placa ESP32-S3-Touch-LCD-2.8B. Cada ambiente (simulador PC
 * ou hardware real) implementa estas estruturas.
 *
 * Dependência: unidirecional (portas → HAL → firmware → UI).
 * A UI nunca acessa hardware diretamente — sempre via HAL.
 * ============================================================ */

/* ---- Tipos de dados do HAL ---- */

/* Horário proveniente do RTC PCF85063 */
typedef struct {
    uint16_t year;     /* 2000-2099 */
    uint8_t  month;    /* 1-12 */
    uint8_t  day;      /* 1-31 */
    uint8_t  hour;     /* 0-23 */
    uint8_t  minute;   /* 0-59 */
    uint8_t  second;   /* 0-59 */
    uint8_t  weekday;  /* 0=domingo, 6=sábado */
} runesos_hal_time_t;

/* Nível de bateria LiPo */
typedef struct {
    uint8_t  percent;       /* 0-100 */
    bool     charging;      /* true se carregando via USB */
    uint16_t voltage_mv;    /* tensão em miliamps (opcional) */
} runesos_hal_battery_t;

/* Estado do rádio (WiFi + Bluetooth) */
typedef struct {
    bool     wifi_connected;
    int8_t   wifi_rssi;       /* dBm: -100 (fraco) a 0 (forte) */
    uint8_t  wifi_signal;     /* 0-4 barras (conveniência p/ UI) */
    bool     bt_enabled;
    bool     bt_connected;    /* teclado pareado */
    char     bt_device_name[32]; /* nome do dispositivo pareado */
} runesos_hal_radio_t;

/* Leitura do IMU QMI8658 (6 eixos) */
typedef struct {
    int16_t accel_x;   /* mg */
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;    /* mdps (miligraus/seg) */
    int16_t gyro_y;
    int16_t gyro_z;
    float   temp_c;     /* temperatura interna do sensor */
} runesos_hal_imu_t;

/* Estado do armazenamento SD */
typedef struct {
    bool     mounted;        /* SD card montado e acessível */
    uint64_t total_bytes;    /* capacidade total */
    uint64_t free_bytes;     /* espaço livre */
    char     mount_point[16]; /* ex: "/sdcard" ou "sdcard/" no PC */
} runesos_hal_sd_t;

/* Estado do áudio */
typedef struct {
    bool     buzzer_active;
    bool     speaker_active;
    uint8_t  volume;         /* 0-100 */
} runesos_hal_audio_t;

/* Callback para buzz do buzzer (timer/alarm) */
typedef void (*runesos_hal_buzz_done_cb_t)(void);

/* ---- Estrutura principal do HAL ---- */
typedef struct {

    /* ---------- Tempo (RTC PCF85063) ---------- */
    void (*get_time)(runesos_hal_time_t *t);
    void (*set_time)(const runesos_hal_time_t *t);  /* só no hardware real */

    /* ---------- Bateria ---------- */
    void (*get_battery)(runesos_hal_battery_t *b);
    uint8_t (*get_battery_percent)(void);  /* conveniência: só o % */

    /* ---------- Rádio (WiFi + BLE) ---------- */
    void (*get_radio)(runesos_hal_radio_t *r);

    /* ---------- IMU (QMI8658 6 eixos) ---------- */
    void (*get_imu)(runesos_hal_imu_t *imu);

    /* ---------- SD Card ---------- */
    void (*get_sd)(runesos_hal_sd_t *sd);

    /* ---------- Áudio (buzzer + speaker) ---------- */
    void (*buzzer_on)(uint16_t freq_hz, uint16_t duration_ms);
    void (*buzzer_off)(void);
    void (*buzzer_beep)(uint16_t freq_hz, uint16_t duration_ms,
                         runesos_hal_buzz_done_cb_t cb);

    void (*set_volume)(uint8_t percent);
    uint8_t (*get_volume)(void);

    /* ---------- Backlight ---------- */
    void (*set_backlight)(uint8_t percent);  /* 0-100 */
    uint8_t (*get_backlight)(void);

    /* ---------- Power ---------- */
    void (*enter_sleep)(void);     /* light sleep */
    void (*wake_up)(void);

} runesos_hal_t;

/* ============================================================
 * Helpers — conversões de conveniência para a UI
 * ============================================================ */

/* Converte RSSI (dBm) para barras (0-4) */
static inline uint8_t runesos_rssi_to_bars(int8_t rssi)
{
    if (rssi >= -55) return 4;
    if (rssi >= -67) return 3;
    if (rssi >= -78) return 2;
    if (rssi >= -90) return 1;
    return 0;
}

/* Converte percent de bateria para ícone (0-4 níveis) */
static inline uint8_t runesos_batt_to_level(uint8_t percent)
{
    if (percent >= 75) return 4;
    if (percent >= 50) return 3;
    if (percent >= 25) return 2;
    if (percent >= 10) return 1;
    return 0;
}

/* Formata tamanho de arquivo (bytes → string legível) */
static inline void runesos_format_size(uint64_t bytes, char *out, size_t len)
{
    if (bytes < 1024) {
        snprintf(out, len, "%llu B", bytes);
    } else if (bytes < 1024 * 1024) {
        snprintf(out, len, "%.1f KB", (float)bytes / 1024.0f);
    } else if (bytes < 1024 * 1024 * 1024) {
        snprintf(out, len, "%.1f MB", (float)bytes / (1024.0f * 1024.0f));
    } else {
        snprintf(out, len, "%.1f GB", (float)bytes / (1024.0f * 1024.0f * 1024.0f));
    }
}

#endif // RUNESOS_HAL_H