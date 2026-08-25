#include "runesos_statusbar.h"
#include "theme_nordico.h"
#include "runesos_quicksettings.h"
#include <stdio.h>

static struct {
    lv_obj_t            *bar;
    lv_obj_t            *time_label;
    lv_obj_t            *battery_label;
    lv_obj_t            *battery_icon;
    lv_obj_t            *wifi_label;
    lv_obj_t            *bt_label;
    lv_timer_t          *timer;
    const runesos_hal_t *hal;
} sb;

/* Escolhe o símbolo de bateria conforme o nível */
static const char *get_battery_symbol(uint8_t level)
{
    if (level >= 90) return LV_SYMBOL_BATTERY_FULL;
    if (level >= 65) return LV_SYMBOL_BATTERY_3;
    if (level >= 35) return LV_SYMBOL_BATTERY_2;
    if (level >= 15) return LV_SYMBOL_BATTERY_1;
    return LV_SYMBOL_BATTERY_EMPTY;
}

static void update_statusbar(void)
{
    if (!sb.hal || !sb.bar) return;

    /* 1. Atualizar Hora */
    runesos_hal_time_t t;
    sb.hal->get_time(&t);
    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
             t.hour, t.minute, t.second);
    if (sb.time_label) {
        lv_label_set_text(sb.time_label, time_str);
    }

    /* 2. Atualizar Bateria */
    runesos_hal_battery_t bat;
    sb.hal->get_battery(&bat);
    if (sb.battery_label && sb.battery_icon) {
        char bat_pct[12];
        snprintf(bat_pct, sizeof(bat_pct), "%d%%", bat.percent);
        lv_label_set_text(sb.battery_label, bat_pct);

        const char *sym = bat.charging ? LV_SYMBOL_CHARGE : get_battery_symbol(bat.percent);
        lv_label_set_text(sb.battery_icon, sym);

        /* Cor conforme estado */
        lv_color_t bat_color = RUNESOS_COLOR_TEXT;
        if (bat.charging) {
            bat_color = RUNESOS_COLOR_ACCENT;
        } else if (bat.percent <= 15) {
            bat_color = lv_color_hex(0xE74C3C); /* Vermelho */
        } else if (bat.percent <= 30) {
            bat_color = lv_color_hex(0xF39C12); /* Laranja */
        }
        lv_obj_set_style_text_color(sb.battery_icon, bat_color, 0);
        lv_obj_set_style_text_color(sb.battery_label, bat_color, 0);
    }

    /* 3. Atualizar Rádio */
    runesos_hal_radio_t radio;
    sb.hal->get_radio(&radio);
    if (sb.wifi_label) {
        lv_color_t wifi_col = radio.wifi_connected ? RUNESOS_COLOR_TEXT : lv_color_hex(0x555566);
        lv_obj_set_style_text_color(sb.wifi_label, wifi_col, 0);
    }
    if (sb.bt_label) {
        lv_color_t bt_col = radio.bt_connected ? RUNESOS_COLOR_TEXT : lv_color_hex(0x555566);
        lv_obj_set_style_text_color(sb.bt_label, bt_col, 0);
    }
}

static void statusbar_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    update_statusbar();
}

static void statusbar_click_cb(lv_event_t *e)
{
    (void)e;
    runesos_quicksettings_toggle(lv_screen_active(), sb.hal);
}

static void statusbar_del_cb(lv_event_t *e)
{
    (void)e;
    if (sb.timer) {
        lv_timer_delete(sb.timer);
        sb.timer = NULL;
    }
    sb.bar = NULL;
    sb.time_label = NULL;
    sb.battery_label = NULL;
    sb.battery_icon = NULL;
    sb.wifi_label = NULL;
    sb.bt_label = NULL;
}

void runesos_statusbar_create(lv_obj_t *parent, const runesos_hal_t *hal)
{
    sb.hal = hal;

    /* Container da Status Bar */
    sb.bar = lv_obj_create(parent);
    lv_obj_set_size(sb.bar, LV_PCT(100), 32);
    lv_obj_align(sb.bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(sb.bar, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(sb.bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(sb.bar, 0, 0);
    lv_obj_set_style_pad_hor(sb.bar, 12, 0);
    lv_obj_set_style_pad_ver(sb.bar, 4, 0);
    lv_obj_set_scrollable(sb.bar, false);

    /* Flex layout horizontal (space-between) */
    lv_obj_set_flex_flow(sb.bar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sb.bar, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Lado Esquerdo: Hora (Dourado) */
    sb.time_label = lv_label_create(sb.bar);
    lv_obj_set_style_text_font(sb.time_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(sb.time_label, RUNESOS_COLOR_ACCENT, 0);
    lv_label_set_text(sb.time_label, "--:--:--");

    /* Lado Direito: Indicadores agrupados */
    lv_obj_t *indicators = lv_obj_create(sb.bar);
    lv_obj_set_style_bg_opa(indicators, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(indicators, 0, 0);
    lv_obj_set_style_pad_all(indicators, 0, 0);
    lv_obj_set_flex_flow(indicators, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(indicators, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(indicators, 8, 0);
    lv_obj_set_scrollable(indicators, false);

    /* Wi-Fi */
    sb.wifi_label = lv_label_create(indicators);
    lv_label_set_text(sb.wifi_label, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_font(sb.wifi_label, &lv_font_montserrat_14, 0);

    /* Bluetooth */
    sb.bt_label = lv_label_create(indicators);
    lv_label_set_text(sb.bt_label, LV_SYMBOL_BLUETOOTH);
    lv_obj_set_style_text_font(sb.bt_label, &lv_font_montserrat_14, 0);

    /* Bateria (Texto % + Ícone) */
    sb.battery_label = lv_label_create(indicators);
    lv_obj_set_style_text_font(sb.battery_label, &lv_font_montserrat_12, 0);
    lv_label_set_text(sb.battery_label, "--%");

    sb.battery_icon = lv_label_create(indicators);
    lv_obj_set_style_text_font(sb.battery_icon, &lv_font_montserrat_14, 0);
    lv_label_set_text(sb.battery_icon, LV_SYMBOL_BATTERY_FULL);

    /* Evento de clique para abrir Quick Settings */
    lv_obj_add_event_cb(sb.bar, statusbar_click_cb, LV_EVENT_CLICKED, NULL);

    /* Cleanup ao deletar */
    lv_obj_add_event_cb(sb.bar, statusbar_del_cb, LV_EVENT_DELETE, NULL);

    /* Atualização inicial imediata */
    update_statusbar();

    /* Timer a 1 Hz */
    sb.timer = lv_timer_create(statusbar_timer_cb, 1000, NULL);
}

