#include "runesos_app.h"
#include "theme_nordico.h"
#include "hal_runesos.h"
#include <stdio.h>
#include <stdlib.h>

extern const runesos_hal_t runesos_hal_mock;

static lv_timer_t *s_monitor_timer = NULL;
static lv_obj_t   *s_ram_bar = NULL;
static lv_obj_t   *s_ram_lbl = NULL;
static lv_obj_t   *s_cpu_lbl = NULL;
static lv_obj_t   *s_temp_lbl = NULL;
static lv_obj_t   *s_fps_lbl = NULL;
static lv_obj_t   *s_opt_lbl = NULL;

typedef struct {
    const char *name;
    const char *state;
    uint8_t     prio;
    uint16_t    stack_free_bytes;
    lv_color_t  state_color;
} task_info_t;

static const task_info_t s_tasks[] = {
    {"gui_render_task",  "EXEC",  5, 4320, { .blue = 0x6B, .green = 0x8E, .red = 0x6B }}, /* Verde */
    {"session_manager",  "PRONTO",4, 3180, { .blue = 0x6B, .green = 0x8E, .red = 0x6B }},
    {"status_service",   "BLOQ",  3, 2850, { .blue = 0x37, .green = 0xAF, .red = 0xD4 }}, /* Dourado */
    {"ble_keyboard_drv", "BLOQ",  4, 3420, { .blue = 0x37, .green = 0xAF, .red = 0xD4 }},
    {"wifi_network_mgr", "BLOQ",  2, 2100, { .blue = 0x37, .green = 0xAF, .red = 0xD4 }},
    {"imu_sensor_poll",  "PRONTO",3, 2560, { .blue = 0x6B, .green = 0x8E, .red = 0x6B }},
    {"idle_task_core0",  "IDLE",  0, 1024, { .blue = 0xA5, .green = 0x6F, .red = 0x4A }},
    {"idle_task_core1",  "IDLE",  0, 1024, { .blue = 0xA5, .green = 0x6F, .red = 0x4A }}
};

#define TOTAL_TASKS (sizeof(s_tasks)/sizeof(s_tasks[0]))

static void update_monitor_cb(lv_timer_t *t)
{
    (void)t;
    if (!s_ram_lbl) return;

    /* Flutuação realista para o monitor */
    int cpu_core0 = 12 + (rand() % 15);
    int cpu_core1 = 4 + (rand() % 8);
    char cpu_txt[64];
    snprintf(cpu_txt, sizeof(cpu_txt), "Core 0: %d%%  |  Core 1: %d%%  (240 MHz)", cpu_core0, cpu_core1);
    lv_label_set_text(s_cpu_lbl, cpu_txt);

    runesos_hal_imu_t imu;
    runesos_hal_mock.get_imu(&imu);
    char temp_txt[32];
    snprintf(temp_txt, sizeof(temp_txt), "Temp: %.1f °C", imu.temp_c);
    lv_label_set_text(s_temp_lbl, temp_txt);
}

static void optimize_mem_btn_cb(lv_event_t *e)
{
    (void)e;
    if (s_opt_lbl) {
        lv_label_set_text(s_opt_lbl, "✨ Cache limpo! 412 KB liberados.");
        lv_obj_set_style_text_color(s_opt_lbl, RUNESOS_COLOR_ACCENT, 0);
    }
    if (s_ram_bar) {
        lv_bar_set_value(s_ram_bar, 14, LV_ANIM_ON);
    }
}

void app_tarefas_create(lv_obj_t *parent)
{
    lv_obj_set_scrollable(parent, false);

    /* Abas: Recursos e Processos */
    lv_obj_t *tabview = lv_tabview_create(parent);
    lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(tabview, RUNESOS_COLOR_BG, 0);

    lv_obj_t *tab_btns = lv_tabview_get_tab_bar(tabview);
    lv_obj_set_style_bg_color(tab_btns, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_text_color(tab_btns, RUNESOS_COLOR_TEXT, 0);

    lv_obj_t *t1 = lv_tabview_add_tab(tabview, "Recursos");
    lv_obj_t *t2 = lv_tabview_add_tab(tabview, "Processos");

    /* ================= Tab 1: Monitor de Recursos ================= */
    lv_obj_set_style_bg_opa(t1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(t1, 10, 0);
    lv_obj_set_style_pad_row(t1, 12, 0);
    lv_obj_set_flex_flow(t1, LV_FLEX_FLOW_COLUMN);

    /* 1. Card CPU */
    lv_obj_t *c_cpu = lv_obj_create(t1);
    lv_obj_set_size(c_cpu, LV_PCT(100), 75);
    lv_obj_set_style_bg_color(c_cpu, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c_cpu, 12, 0);
    lv_obj_set_style_border_width(c_cpu, 1, 0);
    lv_obj_set_style_border_color(c_cpu, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c_cpu, false);

    lv_obj_t *cpu_t = lv_label_create(c_cpu);
    lv_label_set_text(cpu_t, "⚡ Processador ESP32-S3 (Xtensa LX7)");
    lv_obj_set_style_text_font(cpu_t, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(cpu_t, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(cpu_t, LV_ALIGN_TOP_LEFT, 0, 0);

    s_cpu_lbl = lv_label_create(c_cpu);
    lv_label_set_text(s_cpu_lbl, "Core 0: 18%  |  Core 1: 6%  (240 MHz)");
    lv_obj_set_style_text_font(s_cpu_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_cpu_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(s_cpu_lbl, LV_ALIGN_BOTTOM_LEFT, 0, -2);

    /* 2. Card Memória RAM / PSRAM */
    lv_obj_t *c_ram = lv_obj_create(t1);
    lv_obj_set_size(c_ram, LV_PCT(100), 100);
    lv_obj_set_style_bg_color(c_ram, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c_ram, 12, 0);
    lv_obj_set_style_border_width(c_ram, 1, 0);
    lv_obj_set_style_border_color(c_ram, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c_ram, false);

    lv_obj_t *ram_t = lv_label_create(c_ram);
    lv_label_set_text(ram_t, "🧠 Memória Heap (8MB PSRAM + 512KB SRAM)");
    lv_obj_set_style_text_font(ram_t, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(ram_t, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(ram_t, LV_ALIGN_TOP_LEFT, 0, 0);

    s_ram_lbl = lv_label_create(c_ram);
    lv_label_set_text(s_ram_lbl, "Usado: 1.4 MB  |  Livre: 7.1 MB (16%)");
    lv_obj_set_style_text_font(s_ram_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_ram_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(s_ram_lbl, LV_ALIGN_TOP_LEFT, 0, 26);

    s_ram_bar = lv_bar_create(c_ram);
    lv_obj_set_size(s_ram_bar, LV_PCT(100), 10);
    lv_obj_align(s_ram_bar, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_bar_set_value(s_ram_bar, 16, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_ram_bar, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR);

    /* 3. Card Sensores & GUI */
    lv_obj_t *c_sns = lv_obj_create(t1);
    lv_obj_set_size(c_sns, LV_PCT(100), 75);
    lv_obj_set_style_bg_color(c_sns, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c_sns, 12, 0);
    lv_obj_set_style_border_width(c_sns, 1, 0);
    lv_obj_set_style_border_color(c_sns, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c_sns, false);

    s_temp_lbl = lv_label_create(c_sns);
    lv_label_set_text(s_temp_lbl, "Temp: 25.0 °C");
    lv_obj_set_style_text_color(s_temp_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(s_temp_lbl, LV_ALIGN_LEFT_MID, 0, 0);

    s_fps_lbl = lv_label_create(c_sns);
    lv_label_set_text(s_fps_lbl, "Taxa GUI: 60 FPS (ST7701)");
    lv_obj_set_style_text_color(s_fps_lbl, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(s_fps_lbl, LV_ALIGN_RIGHT_MID, 0, 0);

    /* 4. Botão Otimizar Memória */
    lv_obj_t *opt_btn = lv_btn_create(t1);
    lv_obj_set_size(opt_btn, LV_PCT(100), 44);
    lv_obj_set_style_bg_color(opt_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(opt_btn, 10, 0);

    lv_obj_t *opt_btnt = lv_label_create(opt_btn);
    lv_label_set_text(opt_btnt, "🧹 Otimizar Memória / Limpar Cache");
    lv_obj_set_style_text_font(opt_btnt, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(opt_btnt, lv_color_hex(0x1A1A2E), 0);
    lv_obj_center(opt_btnt);
    lv_obj_add_event_cb(opt_btn, optimize_mem_btn_cb, LV_EVENT_CLICKED, NULL);

    s_opt_lbl = lv_label_create(t1);
    lv_label_set_text(s_opt_lbl, "");
    lv_obj_set_style_text_align(s_opt_lbl, LV_TEXT_ALIGN_CENTER, 0);

    /* ================= Tab 2: Processos / Tarefas FreeRTOS ================= */
    lv_obj_set_style_bg_opa(t2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(t2, 6, 0);
    lv_obj_set_style_pad_row(t2, 8, 0);
    lv_obj_set_flex_flow(t2, LV_FLEX_FLOW_COLUMN);

    for (size_t i = 0; i < TOTAL_TASKS; i++) {
        lv_obj_t *item = lv_obj_create(t2);
        lv_obj_set_size(item, LV_PCT(100), 50);
        lv_obj_set_style_bg_color(item, RUNESOS_COLOR_SURFACE, 0);
        lv_obj_set_style_radius(item, 8, 0);
        lv_obj_set_style_border_width(item, 1, 0);
        lv_obj_set_style_border_color(item, lv_color_hex(0x2A3B5C), 0);
        lv_obj_set_style_pad_hor(item, 10, 0);
        lv_obj_set_style_pad_ver(item, 4, 0);
        lv_obj_set_scrollable(item, false);

        lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(item, LV_FLEX_ALIGN_SPACE_BETWEEN,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        /* Nome + Prioridade */
        lv_obj_t *n_box = lv_obj_create(item);
        lv_obj_set_style_bg_opa(n_box, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(n_box, 0, 0);
        lv_obj_set_style_pad_all(n_box, 0, 0);
        lv_obj_set_flex_flow(n_box, LV_FLEX_FLOW_COLUMN);

        lv_obj_t *nl = lv_label_create(n_box);
        lv_label_set_text(nl, s_tasks[i].name);
        lv_obj_set_style_text_font(nl, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(nl, RUNESOS_COLOR_TEXT, 0);

        lv_obj_t *pl = lv_label_create(n_box);
        char p_buf[32];
        snprintf(p_buf, sizeof(p_buf), "Prio: %d | Stack: %u B", s_tasks[i].prio, s_tasks[i].stack_free_bytes);
        lv_label_set_text(pl, p_buf);
        lv_obj_set_style_text_font(pl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(pl, lv_color_hex(0x8888AA), 0);

        /* Badge de Estado */
        lv_obj_t *badge = lv_obj_create(item);
        lv_obj_set_size(badge, 64, 26);
        lv_obj_set_style_bg_color(badge, s_tasks[i].state_color, 0);
        lv_obj_set_style_radius(badge, 6, 0);
        lv_obj_set_style_border_width(badge, 0, 0);
        lv_obj_set_style_pad_all(badge, 0, 0);
        lv_obj_set_scrollable(badge, false);

        lv_obj_t *st_lbl = lv_label_create(badge);
        lv_label_set_text(st_lbl, s_tasks[i].state);
        lv_obj_set_style_text_font(st_lbl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(st_lbl, lv_color_hex(0x101020), 0);
        lv_obj_center(st_lbl);
    }

    s_monitor_timer = lv_timer_create(update_monitor_cb, 1500, NULL);
}

void app_tarefas_destroy(void)
{
    if (s_monitor_timer) {
        lv_timer_delete(s_monitor_timer);
        s_monitor_timer = NULL;
    }
    s_ram_bar = NULL;
    s_ram_lbl = NULL;
    s_cpu_lbl = NULL;
    s_temp_lbl = NULL;
    s_fps_lbl = NULL;
    s_opt_lbl = NULL;
}

