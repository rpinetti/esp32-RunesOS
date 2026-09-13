#include "launcher.h"
#include "../theme/theme_nordico.h"
#include "../core/runesos_app.h"
#include "time_service.h"
#include "battery_service.h"
#include "radio_service.h"
#include <stdio.h>

/* Referências que o timer precisa */
static lv_obj_t *s_time_label = NULL;
static lv_obj_t *s_battery_label = NULL;
static lv_obj_t *s_wifi_label = NULL;
static lv_obj_t *s_bt_label = NULL;

/* Escolhe o símbolo de bateria conforme o nível */
static const char *battery_symbol(int level)
{
    if (level >= 95) return LV_SYMBOL_BATTERY_FULL;
    if (level >= 66) return LV_SYMBOL_BATTERY_3;
    if (level >= 33) return LV_SYMBOL_BATTERY_2;
    if (level >= 5)  return LV_SYMBOL_BATTERY_1;
    return LV_SYMBOL_BATTERY_EMPTY;
}

/* Atualiza o ícone de bateria */
static void update_battery(const runesos_hal_battery_t *bat)
{
    const char *sym = bat->charging ? LV_SYMBOL_CHARGE : battery_symbol(bat->percent);
    lv_label_set_text(s_battery_label, sym);

    /* Vermelho se crítico, dourado se carregando, branco normal */
    lv_color_t color = RUNESOS_COLOR_TEXT;
    if (bat->charging)             color = RUNESOS_COLOR_ACCENT;
    else if (bat->percent <= 15)   color = lv_color_hex(0xe74c3c);
    lv_obj_set_style_text_color(s_battery_label, color, 0);
}

/* Atualiza os ícones de Wi-Fi e BT */
static void update_radio(const runesos_hal_radio_t *radio)
{
    lv_obj_set_style_text_color(s_wifi_label,
        radio->wifi_connected ? RUNESOS_COLOR_TEXT : lv_color_hex(0x555555), 0);
    lv_obj_set_style_text_color(s_bt_label,
        radio->bt_connected ? RUNESOS_COLOR_TEXT : lv_color_hex(0x555555), 0);
}

/* Callback do timer: consulta o firmware e atualiza tudo */
static void statusbar_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    runesos_hal_time_t t;
    runesos_hal_battery_t bat;
    runesos_hal_radio_t radio;
    char buf[16];

    /* Hora */
    runesos_time_get(&t);
    lv_snprintf(buf, sizeof(buf), "%02d:%02d:%02d",
                t.hour, t.minute, t.second);
    lv_label_set_text(s_time_label, buf);

    /* Bateria */
    runesos_battery_get(&bat);
    update_battery(&bat);

    /* Rádio */
    runesos_radio_get(&radio);
    update_radio(&radio);
}

/* Cria a status bar no topo */
static void create_status_bar(lv_obj_t *parent)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, LV_PCT(100), 32);
    lv_obj_align(bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(bar, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_pad_hor(bar, 12, 0);
    lv_obj_set_flex_flow(bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Hora (esquerda, vinda do firmware) */
    s_time_label = lv_label_create(bar);
    lv_obj_set_style_text_color(s_time_label, RUNESOS_COLOR_ACCENT, 0);
    lv_label_set_text(s_time_label, "--:--:--");

    /* Grupo de indicadores (direita) */
    lv_obj_t *indicators = lv_obj_create(bar);
    lv_obj_set_style_bg_color(indicators, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(indicators, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(indicators, 0, 0);
    lv_obj_set_flex_flow(indicators, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(indicators, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(indicators, 8, 0);

    /* Wi-Fi */
    s_wifi_label = lv_label_create(indicators);
    lv_label_set_text(s_wifi_label, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(s_wifi_label, lv_color_hex(0x555555), 0);

    /* Bluetooth */
    s_bt_label = lv_label_create(indicators);
    lv_label_set_text(s_bt_label, LV_SYMBOL_BLUETOOTH);
    lv_obj_set_style_text_color(s_bt_label, lv_color_hex(0x555555), 0);

    /* Bateria */
    s_battery_label = lv_label_create(indicators);
    lv_label_set_text(s_battery_label, LV_SYMBOL_BATTERY_FULL);
    lv_obj_set_style_text_color(s_battery_label, RUNESOS_COLOR_TEXT, 0);
}

/* Cria um "app" placeholder no grid (igual ao anterior) */
static lv_obj_t *create_app_icon(lv_obj_t *parent, const char *name)
{
    lv_obj_t *cell = lv_obj_create(parent);
    lv_obj_set_size(cell, 72, 84);
    lv_obj_set_style_bg_color(cell, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(cell, 0, 0);
    lv_obj_set_style_radius(cell, 12, 0);
    lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cell, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *icon = lv_obj_create(cell);
    lv_obj_set_size(icon, 44, 44);
    lv_obj_set_style_bg_color(icon, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(icon, 10, 0);
    lv_obj_set_style_border_width(icon, 0, 0);

    lv_obj_t *lbl = lv_label_create(cell);
    lv_label_set_text(lbl, name);
    lv_obj_set_style_text_color(lbl, RUNESOS_COLOR_TEXT, 0);

    return cell;
}

lv_obj_t *runesos_launcher_create(void)
{
    lv_obj_t *scr = lv_scr_act();
    runesos_theme_apply(scr);

    /* Status bar (topo) */
    create_status_bar(scr);

    /* Área de apps (grid flexível) */
    lv_obj_t *apps = lv_obj_create(scr);
    lv_obj_set_size(apps, LV_PCT(100), LV_PCT(80));
    lv_obj_align(apps, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_bg_color(apps, RUNESOS_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(apps, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(apps, 0, 0);
    lv_obj_set_flex_flow(apps, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(apps, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(apps, 16, 0);
    lv_obj_set_style_pad_column(apps, 16, 0);

    create_app_icon(apps, "Oráculo");
    create_app_icon(apps, "Notas");
    create_app_icon(apps, "Relógio");
    create_app_icon(apps, "Galeria");
    create_app_icon(apps, "Terminal");
    create_app_icon(apps, "Música");

    /* Dock (base) */
    lv_obj_t *dock = lv_obj_create(scr);
    lv_obj_set_size(dock, LV_PCT(100), 56);
    lv_obj_align(dock, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(dock, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(dock, 0, 0);
    lv_obj_set_flex_flow(dock, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dock, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    create_app_icon(dock, "Telefone");
    create_app_icon(dock, "Mensagens");

    /* Timer: atualiza hora + bateria + rádio a cada 1s */
    lv_timer_create(statusbar_timer_cb, 1000, NULL);

    return scr;
}