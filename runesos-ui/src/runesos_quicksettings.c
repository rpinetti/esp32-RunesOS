#include "runesos_quicksettings.h"
#include "theme_nordico.h"
#include "runesos_session.h"
#include <stdio.h>

static struct {
    lv_obj_t            *panel;
    lv_obj_t            *flashlight_overlay;
    const runesos_hal_t *hal;
    bool                 is_open;
    bool                 flashlight_on;
    bool                 silent_mode;
} qs;

static void close_btn_cb(lv_event_t *e)
{
    (void)e;
    runesos_quicksettings_close();
}

static void lock_btn_cb(lv_event_t *e)
{
    (void)e;
    runesos_quicksettings_close();
    runesos_session_lock();
}

static void flashlight_click_cb(lv_event_t *e)
{
    (void)e;
    qs.flashlight_on = !qs.flashlight_on;
    if (qs.flashlight_on) {
        if (!qs.flashlight_overlay) {
            lv_obj_t *root = lv_screen_active();
            qs.flashlight_overlay = lv_obj_create(root);
            lv_obj_set_size(qs.flashlight_overlay, LV_PCT(100), LV_PCT(100));
            lv_obj_set_pos(qs.flashlight_overlay, 0, 0);
            lv_obj_set_style_bg_color(qs.flashlight_overlay, lv_color_hex(0xFFFFFF), 0);
            lv_obj_set_style_bg_opa(qs.flashlight_overlay, LV_OPA_COVER, 0);
            lv_obj_set_style_border_width(qs.flashlight_overlay, 0, 0);
            lv_obj_set_scrollable(qs.flashlight_overlay, false);

            lv_obj_t *hint = lv_label_create(qs.flashlight_overlay);
            lv_label_set_text(hint, "🔦 Lanterna Ativa\n(Toque para desligar)");
            lv_obj_set_style_text_color(hint, lv_color_hex(0x101010), 0);
            lv_obj_set_style_text_font(hint, &lv_font_montserrat_16, 0);
            lv_obj_set_style_text_align(hint, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_center(hint);

            lv_obj_add_event_cb(qs.flashlight_overlay, flashlight_click_cb, LV_EVENT_CLICKED, NULL);
        }
    } else {
        if (qs.flashlight_overlay) {
            lv_obj_delete(qs.flashlight_overlay);
            qs.flashlight_overlay = NULL;
        }
    }
}

static void bright_slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    if (qs.hal && qs.hal->set_backlight) {
        qs.hal->set_backlight((uint8_t)val);
    }
}

static void vol_slider_cb(lv_event_t *e)
{
    lv_obj_t *slider = lv_event_get_target(e);
    int32_t val = lv_slider_get_value(slider);
    if (qs.hal && qs.hal->set_volume) {
        qs.hal->set_volume((uint8_t)val);
    }
}

static void anim_y_cb(void *var, int32_t v)
{
    lv_obj_set_y((lv_obj_t *)var, v);
}

void runesos_quicksettings_toggle(lv_obj_t *parent, const runesos_hal_t *hal)
{
    if (qs.is_open) {
        runesos_quicksettings_close();
        return;
    }

    qs.hal = hal;
    qs.is_open = true;

    /* Painel Suspenso */
    qs.panel = lv_obj_create(parent);
    lv_obj_set_size(qs.panel, 480, 360);
    lv_obj_set_pos(qs.panel, 0, -360);
    lv_obj_set_style_bg_color(qs.panel, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(qs.panel, LV_OPA_90, 0);
    lv_obj_set_style_border_width(qs.panel, 1, 0);
    lv_obj_set_style_border_color(qs.panel, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(qs.panel, 20, 0);
    lv_obj_set_style_pad_hor(qs.panel, 16, 0);
    lv_obj_set_style_pad_ver(qs.panel, 10, 0);
    lv_obj_set_scrollable(qs.panel, false);

    /* 1. Header: Hora + Símbolo + Fechar */
    lv_obj_t *hdr = lv_obj_create(qs.panel);
    lv_obj_set_size(hdr, LV_PCT(100), 38);
    lv_obj_align(hdr, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_opa(hdr, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(hdr, 0, 0);
    lv_obj_set_style_pad_all(hdr, 0, 0);
    lv_obj_set_scrollable(hdr, false);
    lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(hdr);
    lv_label_set_text(title, "⚡ Central de Controle");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, RUNESOS_COLOR_ACCENT, 0);

    lv_obj_t *close_btn = lv_btn_create(hdr);
    lv_obj_set_size(close_btn, 84, 30);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_radius(close_btn, 6, 0);
    lv_obj_t *cl = lv_label_create(close_btn);
    lv_label_set_text(cl, "▲ Recolher");
    lv_obj_set_style_text_font(cl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(cl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_center(cl);
    lv_obj_add_event_cb(close_btn, close_btn_cb, LV_EVENT_CLICKED, NULL);

    /* 2. Grid de Toggles Rápidos */
    lv_obj_t *toggles_row = lv_obj_create(qs.panel);
    lv_obj_set_size(toggles_row, LV_PCT(100), 75);
    lv_obj_align(toggles_row, LV_ALIGN_TOP_MID, 0, 44);
    lv_obj_set_style_bg_opa(toggles_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(toggles_row, 0, 0);
    lv_obj_set_style_pad_all(toggles_row, 0, 0);
    lv_obj_set_scrollable(toggles_row, false);
    lv_obj_set_flex_flow(toggles_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toggles_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Toggle Wi-Fi */
    lv_obj_t *b_wifi = lv_btn_create(toggles_row);
    lv_obj_set_size(b_wifi, 90, 64);
    lv_obj_set_style_bg_color(b_wifi, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(b_wifi, 12, 0);
    lv_obj_t *l_wifi = lv_label_create(b_wifi);
    lv_label_set_text(l_wifi, LV_SYMBOL_WIFI "\nWi-Fi");
    lv_obj_set_style_text_align(l_wifi, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(l_wifi, lv_color_hex(0x101020), 0);
    lv_obj_center(l_wifi);

    /* Toggle Bluetooth */
    lv_obj_t *b_bt = lv_btn_create(toggles_row);
    lv_obj_set_size(b_bt, 90, 64);
    lv_obj_set_style_bg_color(b_bt, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(b_bt, 12, 0);
    lv_obj_t *l_bt = lv_label_create(b_bt);
    lv_label_set_text(l_bt, LV_SYMBOL_BLUETOOTH "\nBLE");
    lv_obj_set_style_text_align(l_bt, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(l_bt, lv_color_hex(0x101020), 0);
    lv_obj_center(l_bt);

    /* Toggle Lanterna */
    lv_obj_t *b_flash = lv_btn_create(toggles_row);
    lv_obj_set_size(b_flash, 90, 64);
    lv_obj_set_style_bg_color(b_flash, lv_color_hex(0x334466), 0);
    lv_obj_set_style_radius(b_flash, 12, 0);
    lv_obj_t *l_flash = lv_label_create(b_flash);
    lv_label_set_text(l_flash, "🔦\nLanterna");
    lv_obj_set_style_text_align(l_flash, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(l_flash);
    lv_obj_add_event_cb(b_flash, flashlight_click_cb, LV_EVENT_CLICKED, NULL);

    /* Toggle Silencioso */
    lv_obj_t *b_sil = lv_btn_create(toggles_row);
    lv_obj_set_size(b_sil, 90, 64);
    lv_obj_set_style_bg_color(b_sil, lv_color_hex(0x334466), 0);
    lv_obj_set_style_radius(b_sil, 12, 0);
    lv_obj_t *l_sil = lv_label_create(b_sil);
    lv_label_set_text(l_sil, "🌙\nSilencioso");
    lv_obj_set_style_text_align(l_sil, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(l_sil);

    /* 3. Slider Brilho */
    lv_obj_t *br_box = lv_obj_create(qs.panel);
    lv_obj_set_size(br_box, LV_PCT(100), 55);
    lv_obj_align(br_box, LV_ALIGN_TOP_MID, 0, 124);
    lv_obj_set_style_bg_opa(br_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(br_box, 0, 0);
    lv_obj_set_style_pad_all(br_box, 0, 0);
    lv_obj_set_scrollable(br_box, false);

    lv_obj_t *br_lbl = lv_label_create(br_box);
    lv_label_set_text(br_lbl, "🔆 Brilho do Display");
    lv_obj_set_style_text_font(br_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(br_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(br_lbl, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *br_slider = lv_slider_create(br_box);
    lv_slider_set_range(br_slider, 10, 100);
    lv_slider_set_value(br_slider, 80, LV_ANIM_OFF);
    lv_obj_set_size(br_slider, LV_PCT(100), 12);
    lv_obj_align(br_slider, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(br_slider, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(br_slider, RUNESOS_COLOR_ACCENT, LV_PART_KNOB);
    lv_obj_add_event_cb(br_slider, bright_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* 4. Slider Volume */
    lv_obj_t *vol_box = lv_obj_create(qs.panel);
    lv_obj_set_size(vol_box, LV_PCT(100), 55);
    lv_obj_align(vol_box, LV_ALIGN_TOP_MID, 0, 186);
    lv_obj_set_style_bg_opa(vol_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vol_box, 0, 0);
    lv_obj_set_style_pad_all(vol_box, 0, 0);
    lv_obj_set_scrollable(vol_box, false);

    lv_obj_t *vol_lbl = lv_label_create(vol_box);
    lv_label_set_text(vol_lbl, "🔊 Volume do Buzzer");
    lv_obj_set_style_text_font(vol_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(vol_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(vol_lbl, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *vol_slider = lv_slider_create(vol_box);
    lv_slider_set_range(vol_slider, 0, 100);
    lv_slider_set_value(vol_slider, 60, LV_ANIM_OFF);
    lv_obj_set_size(vol_slider, LV_PCT(100), 12);
    lv_obj_align(vol_slider, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(vol_slider, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(vol_slider, RUNESOS_COLOR_ACCENT, LV_PART_KNOB);
    lv_obj_add_event_cb(vol_slider, vol_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);

    /* 5. Ações Rápidas Inferiores */
    lv_obj_t *act_row = lv_obj_create(qs.panel);
    lv_obj_set_size(act_row, LV_PCT(100), 45);
    lv_obj_align(act_row, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(act_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(act_row, 0, 0);
    lv_obj_set_style_pad_all(act_row, 0, 0);
    lv_obj_set_scrollable(act_row, false);
    lv_obj_set_flex_flow(act_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(act_row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *lock_btn = lv_btn_create(act_row);
    lv_obj_set_size(lock_btn, 130, 36);
    lv_obj_set_style_bg_color(lock_btn, lv_color_hex(0x8B2635), 0);
    lv_obj_set_style_radius(lock_btn, 8, 0);
    lv_obj_t *lbl_l = lv_label_create(lock_btn);
    lv_label_set_text(lbl_l, "🔒 Bloquear");
    lv_obj_set_style_text_font(lbl_l, &lv_font_montserrat_12, 0);
    lv_obj_center(lbl_l);
    lv_obj_add_event_cb(lock_btn, lock_btn_cb, LV_EVENT_CLICKED, NULL);

    /* Animação Slide Down */
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, qs.panel);
    lv_anim_set_values(&a, -360, 0);
    lv_anim_set_time(&a, 300);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    lv_anim_start(&a);
}

bool runesos_quicksettings_is_open(void)
{
    return qs.is_open;
}

static void close_anim_done_cb(lv_anim_t *a)
{
    (void)a;
    if (qs.panel) {
        lv_obj_delete(qs.panel);
        qs.panel = NULL;
    }
    qs.is_open = false;
}

void runesos_quicksettings_close(void)
{
    if (!qs.is_open || !qs.panel) return;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, qs.panel);
    lv_anim_set_values(&a, 0, -360);
    lv_anim_set_time(&a, 250);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    lv_anim_set_completed_cb(&a, close_anim_done_cb);
    lv_anim_start(&a);
}

