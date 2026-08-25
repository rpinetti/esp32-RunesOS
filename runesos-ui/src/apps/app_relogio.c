#include "runesos_app.h"
#include "theme_nordico.h"
#include "hal_runesos.h"
#include <stdio.h>

extern const runesos_hal_t runesos_hal_mock;

/* Estado do Relógio */
static lv_obj_t *s_clock_time_lbl = NULL;
static lv_obj_t *s_clock_date_lbl = NULL;
static lv_timer_t *s_clock_timer = NULL;

/* Estado do Cronômetro */
static lv_obj_t *s_chrono_time_lbl = NULL;
static lv_obj_t *s_chrono_start_btn = NULL;
static lv_obj_t *s_chrono_btn_lbl = NULL;
static lv_timer_t *s_chrono_timer = NULL;
static uint32_t  s_chrono_ms = 0;
static bool      s_chrono_running = false;

/* Estado do Timer Regressivo */
static lv_obj_t *s_timer_time_lbl = NULL;
static lv_obj_t *s_timer_status_lbl = NULL;
static lv_timer_t *s_countdown_timer = NULL;
static int32_t   s_countdown_sec = 60; /* 1 minuto inicial */
static bool      s_countdown_running = false;

static const char *s_weekdays_pt[] = {
    "Domingo", "Segunda-feira", "Terça-feira", "Quarta-feira",
    "Quinta-feira", "Sexta-feira", "Sábado"
};

static const char *s_months_pt[] = {
    "janeiro", "fevereiro", "março", "abril", "maio", "junho",
    "julho", "agosto", "setembro", "outubro", "novembro", "dezembro"
};

/* ---- Callbacks do Relógio ---- */
static void clock_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (!s_clock_time_lbl) return;

    runesos_hal_time_t tm;
    runesos_hal_mock.get_time(&tm);

    char time_str[16];
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d",
             tm.hour, tm.minute, tm.second);
    lv_label_set_text(s_clock_time_lbl, time_str);

    char date_str[64];
    snprintf(date_str, sizeof(date_str), "%s\n%d de %s de %04d",
             s_weekdays_pt[tm.weekday % 7],
             tm.day,
             s_months_pt[tm.month % 12],
             tm.year);
    lv_label_set_text(s_clock_date_lbl, date_str);
}

/* ---- Callbacks do Cronômetro ---- */
static void chrono_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (!s_chrono_running || !s_chrono_time_lbl) return;

    s_chrono_ms += 100;
    uint32_t total_sec = s_chrono_ms / 1000;
    uint32_t mins = total_sec / 60;
    uint32_t secs = total_sec % 60;
    uint32_t tenths = (s_chrono_ms % 1000) / 100;

    char buf[32];
    snprintf(buf, sizeof(buf), "%02u:%02u.%u", mins, secs, tenths);
    lv_label_set_text(s_chrono_time_lbl, buf);
}

static void chrono_toggle_cb(lv_event_t *e)
{
    (void)e;
    s_chrono_running = !s_chrono_running;
    if (s_chrono_btn_lbl) {
        lv_label_set_text(s_chrono_btn_lbl, s_chrono_running ? "Pausar" : "Iniciar");
    }
}

static void chrono_reset_cb(lv_event_t *e)
{
    (void)e;
    s_chrono_running = false;
    s_chrono_ms = 0;
    if (s_chrono_time_lbl) {
        lv_label_set_text(s_chrono_time_lbl, "00:00.0");
    }
    if (s_chrono_btn_lbl) {
        lv_label_set_text(s_chrono_btn_lbl, "Iniciar");
    }
}

/* ---- Callbacks do Timer ---- */
static void update_timer_display(void)
{
    if (!s_timer_time_lbl) return;
    int32_t s = s_countdown_sec < 0 ? 0 : s_countdown_sec;
    int32_t m = s / 60;
    int32_t sec = s % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", m, sec);
    lv_label_set_text(s_timer_time_lbl, buf);
}

static void countdown_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (!s_countdown_running) return;

    if (s_countdown_sec > 0) {
        s_countdown_sec--;
        update_timer_display();
    } else {
        s_countdown_running = false;
        if (s_timer_status_lbl) {
            lv_label_set_text(s_timer_status_lbl, "🔔 TEMPO ESGOTADO!");
            lv_obj_set_style_text_color(s_timer_status_lbl, lv_color_hex(0xE74C3C), 0);
        }
        runesos_hal_mock.buzzer_on(2000, 1000);
    }
}

static void timer_add_cb(lv_event_t *e)
{
    uintptr_t add_sec = (uintptr_t)lv_event_get_user_data(e);
    s_countdown_sec += (int32_t)add_sec;
    if (s_timer_status_lbl) {
        lv_label_set_text(s_timer_status_lbl, "Pronto");
        lv_obj_set_style_text_color(s_timer_status_lbl, RUNESOS_COLOR_TEXT, 0);
    }
    update_timer_display();
}

static void timer_start_cb(lv_event_t *e)
{
    (void)e;
    if (s_countdown_sec > 0) {
        s_countdown_running = !s_countdown_running;
        if (s_timer_status_lbl) {
            lv_label_set_text(s_timer_status_lbl, s_countdown_running ? "Em contagem..." : "Pausado");
            lv_obj_set_style_text_color(s_timer_status_lbl, RUNESOS_COLOR_ACCENT, 0);
        }
    }
}

static void timer_reset_cb(lv_event_t *e)
{
    (void)e;
    s_countdown_running = false;
    s_countdown_sec = 60;
    if (s_timer_status_lbl) {
        lv_label_set_text(s_timer_status_lbl, "Pronto");
        lv_obj_set_style_text_color(s_timer_status_lbl, RUNESOS_COLOR_TEXT, 0);
    }
    update_timer_display();
}

void app_relogio_create(lv_obj_t *parent)
{
    lv_obj_set_scrollable(parent, false);

    /* Abas: Relógio, Cronômetro, Timer */
    lv_obj_t *tabview = lv_tabview_create(parent);
    lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(tabview, RUNESOS_COLOR_BG, 0);

    lv_obj_t *tab_btns = lv_tabview_get_tab_bar(tabview);
    lv_obj_set_style_bg_color(tab_btns, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_text_color(tab_btns, RUNESOS_COLOR_TEXT, 0);

    lv_obj_t *t1 = lv_tabview_add_tab(tabview, "Relógio");
    lv_obj_t *t2 = lv_tabview_add_tab(tabview, "Cronômetro");
    lv_obj_t *t3 = lv_tabview_add_tab(tabview, "Timer");

    /* ================= Tab 1: Relógio ================= */
    lv_obj_set_style_bg_opa(t1, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollable(t1, false);

    lv_obj_t *rune_t1 = lv_label_create(t1);
    lv_label_set_text(rune_t1, "ᛏ");
    lv_obj_set_style_text_font(rune_t1, &lv_font_montserrat_36, 0);
    lv_obj_set_style_text_color(rune_t1, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(rune_t1, LV_ALIGN_TOP_MID, 0, 30);

    s_clock_time_lbl = lv_label_create(t1);
    lv_obj_set_style_text_font(s_clock_time_lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(s_clock_time_lbl, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(s_clock_time_lbl, LV_ALIGN_CENTER, 0, -30);
    lv_label_set_text(s_clock_time_lbl, "--:--:--");

    s_clock_date_lbl = lv_label_create(t1);
    lv_obj_set_style_text_font(s_clock_date_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_clock_date_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_set_style_text_align(s_clock_date_lbl, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(s_clock_date_lbl, LV_ALIGN_CENTER, 0, 50);
    lv_label_set_text(s_clock_date_lbl, "");

    s_clock_timer = lv_timer_create(clock_timer_cb, 1000, NULL);
    clock_timer_cb(NULL);

    /* ================= Tab 2: Cronômetro ================= */
    lv_obj_set_style_bg_opa(t2, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollable(t2, false);

    s_chrono_time_lbl = lv_label_create(t2);
    lv_obj_set_style_text_font(s_chrono_time_lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(s_chrono_time_lbl, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(s_chrono_time_lbl, LV_ALIGN_CENTER, 0, -60);
    lv_label_set_text(s_chrono_time_lbl, "00:00.0");

    lv_obj_t *ch_row = lv_obj_create(t2);
    lv_obj_set_size(ch_row, 300, 60);
    lv_obj_align(ch_row, LV_ALIGN_CENTER, 0, 40);
    lv_obj_set_style_bg_opa(ch_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ch_row, 0, 0);
    lv_obj_set_flex_flow(ch_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ch_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    s_chrono_start_btn = lv_btn_create(ch_row);
    lv_obj_set_size(s_chrono_start_btn, 120, 50);
    lv_obj_set_style_bg_color(s_chrono_start_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(s_chrono_start_btn, 12, 0);

    s_chrono_btn_lbl = lv_label_create(s_chrono_start_btn);
    lv_label_set_text(s_chrono_btn_lbl, s_chrono_running ? "Pausar" : "Iniciar");
    lv_obj_set_style_text_color(s_chrono_btn_lbl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_center(s_chrono_btn_lbl);
    lv_obj_add_event_cb(s_chrono_start_btn, chrono_toggle_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *ch_reset_btn = lv_btn_create(ch_row);
    lv_obj_set_size(ch_reset_btn, 120, 50);
    lv_obj_set_style_bg_color(ch_reset_btn, lv_color_hex(0x334466), 0);
    lv_obj_set_style_radius(ch_reset_btn, 12, 0);

    lv_obj_t *rst_lbl = lv_label_create(ch_reset_btn);
    lv_label_set_text(rst_lbl, "Resetar");
    lv_obj_center(rst_lbl);
    lv_obj_add_event_cb(ch_reset_btn, chrono_reset_cb, LV_EVENT_CLICKED, NULL);

    s_chrono_timer = lv_timer_create(chrono_timer_cb, 100, NULL);

    /* ================= Tab 3: Timer Regressivo ================= */
    lv_obj_set_style_bg_opa(t3, LV_OPA_TRANSP, 0);
    lv_obj_set_scrollable(t3, false);

    s_timer_time_lbl = lv_label_create(t3);
    lv_obj_set_style_text_font(s_timer_time_lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(s_timer_time_lbl, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(s_timer_time_lbl, LV_ALIGN_CENTER, 0, -80);
    update_timer_display();

    s_timer_status_lbl = lv_label_create(t3);
    lv_label_set_text(s_timer_status_lbl, "Pronto");
    lv_obj_set_style_text_font(s_timer_status_lbl, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(s_timer_status_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(s_timer_status_lbl, LV_ALIGN_CENTER, 0, -30);

    /* Botões de Adicionar Tempo (+1m, +5m) */
    lv_obj_t *add_row = lv_obj_create(t3);
    lv_obj_set_size(add_row, 360, 50);
    lv_obj_align(add_row, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_opa(add_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(add_row, 0, 0);
    lv_obj_set_flex_flow(add_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(add_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *b1 = lv_btn_create(add_row);
    lv_obj_set_size(b1, 90, 40);
    lv_obj_set_style_bg_color(b1, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_t *l1 = lv_label_create(b1);
    lv_label_set_text(l1, "+1 min");
    lv_obj_center(l1);
    lv_obj_add_event_cb(b1, timer_add_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)60);

    lv_obj_t *b5 = lv_btn_create(add_row);
    lv_obj_set_size(b5, 90, 40);
    lv_obj_set_style_bg_color(b5, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_t *l5 = lv_label_create(b5);
    lv_label_set_text(l5, "+5 min");
    lv_obj_center(l5);
    lv_obj_add_event_cb(b5, timer_add_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)300);

    lv_obj_t *b10 = lv_btn_create(add_row);
    lv_obj_set_size(b10, 90, 40);
    lv_obj_set_style_bg_color(b10, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_t *l10 = lv_label_create(b10);
    lv_label_set_text(l10, "+10 min");
    lv_obj_center(l10);
    lv_obj_add_event_cb(b10, timer_add_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)600);

    /* Ações Iniciar/Resetar */
    lv_obj_t *t_act_row = lv_obj_create(t3);
    lv_obj_set_size(t_act_row, 300, 60);
    lv_obj_align(t_act_row, LV_ALIGN_CENTER, 0, 90);
    lv_obj_set_style_bg_opa(t_act_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(t_act_row, 0, 0);
    lv_obj_set_flex_flow(t_act_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(t_act_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *t_start = lv_btn_create(t_act_row);
    lv_obj_set_size(t_start, 120, 50);
    lv_obj_set_style_bg_color(t_start, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(t_start, 12, 0);
    lv_obj_t *tl = lv_label_create(t_start);
    lv_label_set_text(tl, "Iniciar");
    lv_obj_set_style_text_color(tl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_center(tl);
    lv_obj_add_event_cb(t_start, timer_start_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *t_reset = lv_btn_create(t_act_row);
    lv_obj_set_size(t_reset, 120, 50);
    lv_obj_set_style_bg_color(t_reset, lv_color_hex(0x334466), 0);
    lv_obj_set_style_radius(t_reset, 12, 0);
    lv_obj_t *trl = lv_label_create(t_reset);
    lv_label_set_text(trl, "Resetar");
    lv_obj_center(trl);
    lv_obj_add_event_cb(t_reset, timer_reset_cb, LV_EVENT_CLICKED, NULL);

    s_countdown_timer = lv_timer_create(countdown_timer_cb, 1000, NULL);
}

void app_relogio_destroy(void)
{
    if (s_clock_timer) {
        lv_timer_delete(s_clock_timer);
        s_clock_timer = NULL;
    }
    if (s_chrono_timer) {
        lv_timer_delete(s_chrono_timer);
        s_chrono_timer = NULL;
    }
    if (s_countdown_timer) {
        lv_timer_delete(s_countdown_timer);
        s_countdown_timer = NULL;
    }

    s_clock_time_lbl = NULL;
    s_clock_date_lbl = NULL;
    s_chrono_time_lbl = NULL;
    s_chrono_start_btn = NULL;
    s_chrono_btn_lbl = NULL;
    s_timer_time_lbl = NULL;
    s_timer_status_lbl = NULL;
}

