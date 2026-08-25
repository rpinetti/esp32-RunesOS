#include "runesos_homescreen.h"
#include "runesos_statusbar.h"
#include "runesos_app.h"
#include "theme_nordico.h"
#include "lvgl.h"
#include <stdio.h>

static void (*s_launch_cb)(uint8_t index) = NULL;

static void app_btn_click_cb(lv_event_t *e)
{
    uintptr_t index = (uintptr_t)lv_event_get_user_data(e);
    if (s_launch_cb) {
        s_launch_cb((uint8_t)index);
    }
}

static lv_obj_t *create_app_card(lv_obj_t *parent, const runesos_app_t *app, uint8_t index)
{
    /* Botão/Card do App */
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 120, 120);
    lv_obj_set_style_bg_color(btn, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_80, 0);
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_color(btn, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_pad_all(btn, 8, 0);
    lv_obj_set_style_shadow_width(btn, 10, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_30, 0);
    lv_obj_set_style_shadow_color(btn, lv_color_hex(0x050510), 0);

    /* Flex column: ícone rúnico em cima, nome embaixo */
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Container do ícone */
    lv_obj_t *icon_box = lv_obj_create(btn);
    lv_obj_set_size(icon_box, 56, 56);
    lv_obj_set_style_bg_color(icon_box, app->icon_color, 0);
    lv_obj_set_style_bg_opa(icon_box, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(icon_box, 14, 0);
    lv_obj_set_style_border_width(icon_box, 0, 0);
    lv_obj_set_style_pad_all(icon_box, 0, 0);
    lv_obj_set_scrollable(icon_box, false);

    /* Símbolo Rúnico dentro da caixa */
    lv_obj_t *rune_lbl = lv_label_create(icon_box);
    lv_label_set_text(rune_lbl, app->rune ? app->rune : "ᛟ");
    lv_obj_set_style_text_font(rune_lbl, &lv_font_montserrat_26, 0);
    lv_obj_set_style_text_color(rune_lbl, lv_color_hex(0x101020), 0);
    lv_obj_center(rune_lbl);

    /* Nome do App */
    lv_obj_t *name_lbl = lv_label_create(btn);
    lv_label_set_text(name_lbl, app->name);
    lv_obj_set_style_text_font(name_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(name_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_set_style_pad_top(name_lbl, 4, 0);

    /* Callback de clique */
    lv_obj_add_event_cb(btn, app_btn_click_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)index);

    return btn;
}

void runesos_homescreen_create(lv_obj_t *root,
                               const runesos_hal_t *hal,
                               void (*on_app_launch)(uint8_t index))
{
    s_launch_cb = on_app_launch;

    lv_obj_t *screen = root;
    lv_obj_set_style_bg_color(screen, RUNESOS_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_clean(screen);
    lv_obj_set_scrollable(screen, false);

    /* 1. Status Bar no topo */
    runesos_statusbar_create(screen, hal);

    /* 2. Área principal de Aplicativos (Grid com scroll) */
    lv_obj_t *apps_container = lv_obj_create(screen);
    lv_obj_set_size(apps_container, 480, 500);
    lv_obj_align(apps_container, LV_ALIGN_TOP_MID, 0, 36);
    lv_obj_set_style_bg_opa(apps_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(apps_container, 0, 0);
    lv_obj_set_style_pad_all(apps_container, 16, 0);
    lv_obj_set_style_pad_row(apps_container, 16, 0);
    lv_obj_set_style_pad_column(apps_container, 20, 0);

    /* Flex Grid com Wrap */
    lv_obj_set_flex_flow(apps_container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(apps_container, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    /* Obter lista de apps registrados */
    uint8_t app_count = 0;
    const runesos_app_t *apps = runesos_app_registry_get(&app_count);

    for (uint8_t i = 0; i < app_count; i++) {
        create_app_card(apps_container, &apps[i], i);
    }

    /* 3. Dock Inferior Fixo */
    lv_obj_t *dock = lv_obj_create(screen);
    lv_obj_set_size(dock, 460, 80);
    lv_obj_align(dock, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(dock, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_bg_opa(dock, LV_OPA_90, 0);
    lv_obj_set_style_radius(dock, 24, 0);
    lv_obj_set_style_border_width(dock, 1, 0);
    lv_obj_set_style_border_color(dock, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_pad_hor(dock, 16, 0);
    lv_obj_set_style_pad_ver(dock, 8, 0);
    lv_obj_set_scrollable(dock, false);

    lv_obj_set_flex_flow(dock, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dock, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Dock Apps: Atalhos rápidos (ex: Oráculo, Relógio, Notas, Terminal) */
    uint8_t dock_indices[] = {0, 1, 2, 3};
    for (size_t d = 0; d < sizeof(dock_indices)/sizeof(dock_indices[0]); d++) {
        uint8_t idx = dock_indices[d];
        if (idx < app_count) {
            lv_obj_t *d_btn = lv_btn_create(dock);
            lv_obj_set_size(d_btn, 56, 56);
            lv_obj_set_style_bg_color(d_btn, apps[idx].icon_color, 0);
            lv_obj_set_style_radius(d_btn, 16, 0);
            lv_obj_set_style_border_width(d_btn, 0, 0);
            lv_obj_set_style_pad_all(d_btn, 0, 0);

            lv_obj_t *d_lbl = lv_label_create(d_btn);
            lv_label_set_text(d_lbl, apps[idx].rune ? apps[idx].rune : "ᛟ");
            lv_obj_set_style_text_font(d_lbl, &lv_font_montserrat_24, 0);
            lv_obj_set_style_text_color(d_lbl, lv_color_hex(0x101020), 0);
            lv_obj_center(d_lbl);

            lv_obj_add_event_cb(d_btn, app_btn_click_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)idx);
        }
    }
}

