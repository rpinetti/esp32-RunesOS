#include "runesos_app.h"
#include "theme_nordico.h"
#include "hal_runesos.h"
#include <stdio.h>

extern const runesos_hal_t runesos_hal_mock;

static lv_obj_t *s_bright_val_lbl = NULL;
static lv_obj_t *s_vol_val_lbl = NULL;

static void bright_slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    if (s_bright_val_lbl) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", (int)val);
        lv_label_set_text(s_bright_val_lbl, buf);
    }
    if (runesos_hal_mock.set_backlight) {
        runesos_hal_mock.set_backlight((uint8_t)val);
    }
}

static void vol_slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    if (s_vol_val_lbl) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d%%", (int)val);
        lv_label_set_text(s_vol_val_lbl, buf);
    }
    if (runesos_hal_mock.set_volume) {
        runesos_hal_mock.set_volume((uint8_t)val);
    }
}

void app_config_create(lv_obj_t *parent)
{
    /* Container com scroll para caber todas as opções */
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_row(parent, 14, 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    /* ---- 1. Seção Brilho da Tela ---- */
    lv_obj_t *c1 = lv_obj_create(parent);
    lv_obj_set_size(c1, LV_PCT(100), 80);
    lv_obj_set_style_bg_color(c1, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c1, 12, 0);
    lv_obj_set_style_border_width(c1, 1, 0);
    lv_obj_set_style_border_color(c1, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c1, false);

    lv_obj_t *l1 = lv_label_create(c1);
    lv_label_set_text(l1, "🔆 Brilho do Visor");
    lv_obj_set_style_text_font(l1, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(l1, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(l1, LV_ALIGN_TOP_LEFT, 0, 0);

    s_bright_val_lbl = lv_label_create(c1);
    lv_label_set_text(s_bright_val_lbl, "80%");
    lv_obj_set_style_text_color(s_bright_val_lbl, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(s_bright_val_lbl, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_obj_t *s1 = lv_slider_create(c1);
    lv_slider_set_range(s1, 10, 100);
    lv_slider_set_value(s1, 80, LV_ANIM_OFF);
    lv_obj_set_size(s1, LV_PCT(100), 12);
    lv_obj_align(s1, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(s1, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s1, RUNESOS_COLOR_ACCENT, LV_PART_KNOB);
    lv_obj_add_event_cb(s1, bright_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* ---- 2. Seção Volume de Áudio ---- */
    lv_obj_t *c2 = lv_obj_create(parent);
    lv_obj_set_size(c2, LV_PCT(100), 80);
    lv_obj_set_style_bg_color(c2, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c2, 12, 0);
    lv_obj_set_style_border_width(c2, 1, 0);
    lv_obj_set_style_border_color(c2, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c2, false);

    lv_obj_t *l2 = lv_label_create(c2);
    lv_label_set_text(l2, "🔊 Volume do Buzzer");
    lv_obj_set_style_text_font(l2, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(l2, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(l2, LV_ALIGN_TOP_LEFT, 0, 0);

    s_vol_val_lbl = lv_label_create(c2);
    lv_label_set_text(s_vol_val_lbl, "60%");
    lv_obj_set_style_text_color(s_vol_val_lbl, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(s_vol_val_lbl, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_obj_t *s2 = lv_slider_create(c2);
    lv_slider_set_range(s2, 0, 100);
    lv_slider_set_value(s2, 60, LV_ANIM_OFF);
    lv_obj_set_size(s2, LV_PCT(100), 12);
    lv_obj_align(s2, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(s2, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s2, RUNESOS_COLOR_ACCENT, LV_PART_KNOB);
    lv_obj_add_event_cb(s2, vol_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* ---- 3. Conexões de Rádio ---- */
    lv_obj_t *c3 = lv_obj_create(parent);
    lv_obj_set_size(c3, LV_PCT(100), 100);
    lv_obj_set_style_bg_color(c3, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c3, 12, 0);
    lv_obj_set_style_border_width(c3, 1, 0);
    lv_obj_set_style_border_color(c3, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c3, false);

    /* Linha Wi-Fi */
    lv_obj_t *wl = lv_label_create(c3);
    lv_label_set_text(wl, LV_SYMBOL_WIFI "  Wi-Fi");
    lv_obj_align(wl, LV_ALIGN_TOP_LEFT, 0, 5);

    lv_obj_t *sw1 = lv_switch_create(c3);
    lv_obj_set_size(sw1, 45, 24);
    lv_obj_align(sw1, LV_ALIGN_TOP_RIGHT, 0, 2);
    lv_obj_add_state(sw1, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw1, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR | LV_STATE_CHECKED);

    /* Linha Bluetooth */
    lv_obj_t *bl = lv_label_create(c3);
    lv_label_set_text(bl, LV_SYMBOL_BLUETOOTH "  Bluetooth (Teclado)");
    lv_obj_align(bl, LV_ALIGN_BOTTOM_LEFT, 0, -5);

    lv_obj_t *sw2 = lv_switch_create(c3);
    lv_obj_set_size(sw2, 45, 24);
    lv_obj_align(sw2, LV_ALIGN_BOTTOM_RIGHT, 0, -3);
    lv_obj_add_state(sw2, LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw2, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR | LV_STATE_CHECKED);

    /* ---- 4. Sobre o RunesOS ---- */
    lv_obj_t *c4 = lv_obj_create(parent);
    lv_obj_set_size(c4, LV_PCT(100), 160);
    lv_obj_set_style_bg_color(c4, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c1, 12, 0);
    lv_obj_set_style_border_width(c4, 1, 0);
    lv_obj_set_style_border_color(c4, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c4, false);

    lv_obj_t *info_title = lv_label_create(c4);
    lv_label_set_text(info_title, "ᛟ RunesOS v0.2");
    lv_obj_set_style_text_font(info_title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(info_title, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(info_title, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *info_body = lv_label_create(c4);
    lv_label_set_text(info_body,
        "Cyberdeck Barkley's Tin\n"
        "• MCU: ESP32-S3 (8MB PSRAM, 16MB Flash)\n"
        "• Display: 480x640 RGB ST7701\n"
        "• Touch: GT911 Capacitivo\n"
        "• Licença: GPL-3.0");
    lv_obj_set_style_text_font(info_body, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(info_body, lv_color_hex(0xAAAAAA), 0);
    lv_obj_align(info_body, LV_ALIGN_TOP_LEFT, 0, 30);
}

void app_config_destroy(void)
{
    s_bright_val_lbl = NULL;
    s_vol_val_lbl = NULL;
}

