#include "runesos_app.h"
#include "theme_nordico.h"
#include "hal_runesos.h"
#include "runesos_session.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TERM_BUFFER_SIZE 4096
#define CMD_MAX_LEN      128

static char s_terminal_buffer[TERM_BUFFER_SIZE];
static lv_obj_t *s_log_ta = NULL;
static lv_obj_t *s_cmd_ta = NULL;
static lv_obj_t *s_keyboard = NULL;

extern const runesos_hal_t runesos_hal_mock;

static void term_append(const char *str)
{
    size_t cur_len = strlen(s_terminal_buffer);
    size_t str_len = strlen(str);
    if (cur_len + str_len < TERM_BUFFER_SIZE - 2) {
        strcat(s_terminal_buffer, str);
    } else {
        /* Buffer cheio: limpa metade inicial */
        memmove(s_terminal_buffer, s_terminal_buffer + TERM_BUFFER_SIZE / 2, TERM_BUFFER_SIZE / 2);
        s_terminal_buffer[TERM_BUFFER_SIZE / 2] = '\0';
        strcat(s_terminal_buffer, "\n[...buffer rolado...]\n");
        strcat(s_terminal_buffer, str);
    }
    if (s_log_ta) {
        lv_textarea_set_text(s_log_ta, s_terminal_buffer);
        lv_textarea_set_cursor_pos(s_log_ta, LV_TEXTAREA_CURSOR_LAST);
    }
}

static void execute_command(const char *cmd)
{
    char out[512];
    snprintf(out, sizeof(out), "\n# %s\n", cmd);
    term_append(out);

    /* Pular espaços em branco iniciais */
    while (*cmd == ' ') cmd++;

    if (strlen(cmd) == 0) {
        return;
    }

    if (strcmp(cmd, "help") == 0) {
        term_append("Comandos disponíveis:\n"
                    "  help       - Exibe esta lista de ajuda\n"
                    "  sysinfo    - Informações do sistema e hardware\n"
                    "  time       - Data e hora atual do RTC\n"
                    "  batt       - Status da bateria e tensão\n"
                    "  radio      - Status do Wi-Fi e Bluetooth\n"
                    "  oraculo    - Sorteia uma runa do Elder Futhark\n"
                    "  echo <msg> - Imprime uma mensagem\n"
                    "  lock       - Bloqueia o dispositivo\n"
                    "  clear      - Limpa o terminal\n");
    } else if (strcmp(cmd, "sysinfo") == 0 || strcmp(cmd, "uname") == 0) {
        term_append("╭──────────────────────────────╮\n"
                    "│ RunesOS v0.2-Cyberdeck Tin  │\n"
                    "│ MCU: ESP32-S3 (Dual-Core)   │\n"
                    "│ PSRAM: 8MB | Flash: 16MB    │\n"
                    "│ Display: 480x640 ST7701 RGB │\n"
                    "│ Touch: GT911 Capacitivo     │\n"
                    "│ GUI: LVGL 9.x + FreeRTOS    │\n"
                    "╰──────────────────────────────╯\n");
    } else if (strcmp(cmd, "time") == 0 || strcmp(cmd, "date") == 0) {
        runesos_hal_time_t t;
        runesos_hal_mock.get_time(&t);
        snprintf(out, sizeof(out), "RTC: %02d/%02d/%04d %02d:%02d:%02d\n",
                 t.day, t.month, t.year, t.hour, t.minute, t.second);
        term_append(out);
    } else if (strcmp(cmd, "batt") == 0 || strcmp(cmd, "battery") == 0) {
        runesos_hal_battery_t b;
        runesos_hal_mock.get_battery(&b);
        snprintf(out, sizeof(out), "Bateria: %d%% | %d mV | Carregando: %s\n",
                 b.percent, b.voltage_mv, b.charging ? "SIM" : "NAO");
        term_append(out);
    } else if (strcmp(cmd, "radio") == 0 || strcmp(cmd, "wifi") == 0) {
        runesos_hal_radio_t r;
        runesos_hal_mock.get_radio(&r);
        snprintf(out, sizeof(out), "Wi-Fi: %s (RSSI: %d dBm, %d barras)\n"
                                   "BLE: %s (Disp: %s)\n",
                 r.wifi_connected ? "Conectado" : "Desconectado",
                 r.wifi_rssi, r.wifi_signal,
                 r.bt_connected ? "Pareado" : "Desconectado",
                 r.bt_device_name);
        term_append(out);
    } else if (strcmp(cmd, "oraculo") == 0 || strcmp(cmd, "runa") == 0) {
        const char *runas[] = {
            "ᚠ Fehu (Riqueza)", "ᚢ Uruz (Forca)", "ᚦ Thurisaz (Defesa)",
            "ᚨ Ansuz (Sabedoria)", "ᚱ Raidho (Jornada)", "ᚲ Kenaz (Criatividade)",
            "ᚷ Gebo (Uniao)", "ᚹ Wunjo (Alegria)", "ᚺ Hagalaz (Transformacao)",
            "ᛋ Sowilo (Sucesso)", "ᛏ Tiwaz (Justica)", "ᛒ Berkano (Renascimento)"
        };
        int idx = rand() % (sizeof(runas)/sizeof(runas[0]));
        snprintf(out, sizeof(out), "Oraculo sussurra: %s\n", runas[idx]);
        term_append(out);
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        snprintf(out, sizeof(out), "%s\n", cmd + 5);
        term_append(out);
    } else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) {
        s_terminal_buffer[0] = '\0';
        term_append("=== RunesOS Terminal ===\n");
    } else if (strcmp(cmd, "lock") == 0) {
        term_append("Bloqueando dispositivo...\n");
        runesos_session_lock();
    } else {
        snprintf(out, sizeof(out), "Comando '%s' nao reconhecido. Digite 'help'.\n", cmd);
        term_append(out);
    }
}

static void cmd_submit_cb(lv_event_t *e)
{
    (void)e;
    if (!s_cmd_ta) return;
    const char *text = lv_textarea_get_text(s_cmd_ta);
    if (text && strlen(text) > 0) {
        execute_command(text);
        lv_textarea_set_text(s_cmd_ta, "");
    }
}

static void cmd_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_READY) {
        cmd_submit_cb(e);
    }
}

void app_terminal_create(lv_obj_t *parent)
{
    lv_obj_set_scrollable(parent, false);

    if (strlen(s_terminal_buffer) == 0) {
        snprintf(s_terminal_buffer, sizeof(s_terminal_buffer),
                 "=== RunesOS Terminal ===\n"
                 "Digite 'help' para listar os comandos.\n");
    }

    /* 1. Área de Log / Saída do Terminal */
    s_log_ta = lv_textarea_create(parent);
    lv_obj_set_size(s_log_ta, LV_PCT(100), 240);
    lv_obj_align(s_log_ta, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(s_log_ta, lv_color_hex(0x0E141E), 0);
    lv_obj_set_style_text_color(s_log_ta, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_text_font(s_log_ta, &lv_font_montserrat_12, 0);
    lv_obj_set_style_border_color(s_log_ta, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_border_width(s_log_ta, 1, 0);
    lv_textarea_set_text(s_log_ta, s_terminal_buffer);
    lv_textarea_set_cursor_pos(s_log_ta, LV_TEXTAREA_CURSOR_LAST);

    /* 2. Barra de Entrada do Comando */
    lv_obj_t *input_row = lv_obj_create(parent);
    lv_obj_set_size(input_row, LV_PCT(100), 44);
    lv_obj_align(input_row, LV_ALIGN_TOP_MID, 0, 246);
    lv_obj_set_style_bg_opa(input_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(input_row, 0, 0);
    lv_obj_set_style_pad_all(input_row, 0, 0);
    lv_obj_set_scrollable(input_row, false);

    s_cmd_ta = lv_textarea_create(input_row);
    lv_obj_set_size(s_cmd_ta, 380, 40);
    lv_obj_align(s_cmd_ta, LV_ALIGN_LEFT_MID, 0, 0);
    lv_textarea_set_one_line(s_cmd_ta, true);
    lv_textarea_set_placeholder_text(s_cmd_ta, "Digite um comando...");
    lv_obj_set_style_bg_color(s_cmd_ta, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_text_color(s_cmd_ta, RUNESOS_COLOR_TEXT, 0);
    lv_obj_set_style_border_color(s_cmd_ta, RUNESOS_COLOR_ACCENT, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(s_cmd_ta, cmd_event_cb, LV_EVENT_READY, NULL);

    lv_obj_t *send_btn = lv_btn_create(input_row);
    lv_obj_set_size(send_btn, 68, 40);
    lv_obj_align(send_btn, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(send_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(send_btn, 8, 0);

    lv_obj_t *btn_lbl = lv_label_create(send_btn);
    lv_label_set_text(btn_lbl, "Enviar");
    lv_obj_set_style_text_font(btn_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(btn_lbl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_center(btn_lbl);
    lv_obj_add_event_cb(send_btn, cmd_submit_cb, LV_EVENT_CLICKED, NULL);

    /* 3. Teclado Virtual Integrado */
    s_keyboard = lv_keyboard_create(parent);
    lv_obj_set_size(s_keyboard, LV_PCT(100), 220);
    lv_obj_align(s_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(s_keyboard, s_cmd_ta);
    lv_obj_set_style_bg_color(s_keyboard, RUNESOS_COLOR_SURFACE, 0);
}

void app_terminal_destroy(void)
{
    s_log_ta = NULL;
    s_cmd_ta = NULL;
    s_keyboard = NULL;
}

