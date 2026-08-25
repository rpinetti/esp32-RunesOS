#include "runesos_lockscreen.h"
#include <stdio.h>
#include <string.h>

/* Tema nórdico */
#define COLOR_NIGHT      lv_color_hex(0x1A1A2E)
#define COLOR_GOLD       lv_color_hex(0xD4AF37)
#define COLOR_TEXT_LIGHT lv_color_hex(0xE0E0E0)
#define COLOR_TEXT_DIM   lv_color_hex(0x8888AA)
#define COLOR_OVERLAY    lv_color_hex(0x0D0D1A)

/* Estado interno do lock screen */
static struct {
    lv_obj_t      *root;
    lv_obj_t      *clock_label;
    lv_obj_t      *date_label;
    lv_obj_t      *unlock_hint;
    lv_obj_t      *valknut_label;   /* Símbolo nórdico */
    lv_obj_t      *battery_label;
    lv_timer_t    *timer;
    const runesos_hal_t *hal;
    runesos_unlock_cb_t unlock_cb;
    bool           active;
} ls;

static const char *weekdays_pt[] = {
    "Domingo", "Segunda-feira", "Terça-feira", "Quarta-feira",
    "Quinta-feira", "Sexta-feira", "Sábado"
};

static const char *months_pt[] = {
    "janeiro", "fevereiro", "março", "abril", "maio", "junho",
    "julho", "agosto", "setembro", "outubro", "novembro", "dezembro"
};

/* ---- Atualização do relógio (1 Hz) ---- */
static void update_time_cb(lv_timer_t *t)
{
    (void)t;
    if (!ls.active || !ls.hal)
        return;

    runesos_hal_time_t tm;
    ls.hal->get_time(&tm);

    char clock_str[16];
    snprintf(clock_str, sizeof(clock_str), "%02d:%02d",
             tm.hour, tm.minute);
    lv_label_set_text(ls.clock_label, clock_str);

    char date_str[48];
    snprintf(date_str, sizeof(date_str), "%s, %d de %s",
             weekdays_pt[tm.weekday % 7],
             tm.day,
             months_pt[tm.month % 12]);
    lv_label_set_text(ls.date_label, date_str);

    /* Bateria na lock screen */
    if (ls.battery_label) {
        uint8_t pct = ls.hal->get_battery_percent();
        char bat_str[16];
        snprintf(bat_str, sizeof(bat_str), "⚡ %d%%", pct);
        lv_label_set_text(ls.battery_label, bat_str);
    }
}

/* ---- Animação de desbloqueio (slide up + fade) ---- */
static void unlock_anim_cb(void *var, int32_t v)
{
    lv_obj_t *root = (lv_obj_t *)var;
    int32_t max_h = lv_display_get_vertical_resolution(NULL);

    /* v vai de 0 a 255; mapeia para pixels e opacidade */
    int32_t offset = (int32_t)((float)v / 255.0f * (float)max_h);
    lv_obj_set_y(root, -offset);
    lv_obj_set_style_opa(root, 255 - v, 0);
}

static void unlock_done_cb(lv_anim_t *a)
{
    (void)a;
    ls.active = false;

    if (ls.timer) {
        lv_timer_delete(ls.timer);
        ls.timer = NULL;
    }
    if (ls.root) {
        lv_obj_delete(ls.root);
        ls.root = NULL;
    }
    if (ls.unlock_cb) {
        ls.unlock_cb();
    }
}

static void trigger_unlock(void)
{
    if (!ls.active)
        return;

    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, ls.root);
    lv_anim_set_values(&a, 0, 255);
    lv_anim_set_time(&a, 500);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&a, unlock_anim_cb);
    lv_anim_set_completed_cb(&a, unlock_done_cb);
    lv_anim_start(&a);
}

/* ---- Eventos de toque/gesto ---- */
static void screen_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    /* Tap único desbloqueia (simples para simulador) */
    if (code == LV_EVENT_SHORT_CLICKED) {
        trigger_unlock();
        return;
    }

    /* Swipe para cima desbloqueia (gesto de celular) */
    if (code == LV_EVENT_SCROLL_BEGIN) {
        lv_dir_t dir = lv_indev_get_scroll_dir(lv_indev_active());
        if (dir == LV_DIR_TOP) {
            trigger_unlock();
        }
    }
}

/* ---- Criação da lock screen ---- */
void runesos_lockscreen_create(lv_obj_t *parent,
                                const runesos_hal_t *hal,
                                runesos_unlock_cb_t on_unlock)
{
    if (ls.active)
        return;

    ls.hal = hal;
    ls.unlock_cb = on_unlock;
    ls.active = true;

    /* Container fullscreen */
    ls.root = lv_obj_create(parent);
    lv_obj_set_size(ls.root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(ls.root, 0, 0);
    lv_obj_set_scrollable(ls.root, false);
    lv_obj_set_style_bg_color(ls.root, COLOR_NIGHT, 0);
    lv_obj_set_style_bg_opa(ls.root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ls.root, 0, 0);
    lv_obj_set_style_pad_all(ls.root, 0, 0);

    /* Fundo simples e estável no simulador */
    lv_obj_set_style_bg_color(ls.root, COLOR_NIGHT, 0);

    /* Valknut (símbolo nórdico) no topo */
    ls.valknut_label = lv_label_create(ls.root);
    lv_label_set_text(ls.valknut_label, "ᛟ");  /* Runa Othala */
    lv_obj_set_style_text_font(ls.valknut_label,
        &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(ls.valknut_label, COLOR_GOLD, 0);
    lv_obj_align(ls.valknut_label, LV_ALIGN_TOP_MID, 0, 40);

    /* Relógio grande (centro) */
    ls.clock_label = lv_label_create(ls.root);
    lv_label_set_text(ls.clock_label, "00:00");
    lv_obj_set_style_text_font(ls.clock_label,
        &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(ls.clock_label, COLOR_TEXT_LIGHT, 0);
    lv_obj_align(ls.clock_label, LV_ALIGN_CENTER, 0, -30);

    /* Data abaixo do relógio */
    ls.date_label = lv_label_create(ls.root);
    lv_label_set_text(ls.date_label, "");
    lv_obj_set_style_text_font(ls.date_label,
        &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(ls.date_label, COLOR_TEXT_DIM, 0);
    lv_obj_align(ls.date_label, LV_ALIGN_CENTER, 0, 20);

    /* Bateria (canto inferior esquerdo) */
    ls.battery_label = lv_label_create(ls.root);
    lv_label_set_text(ls.battery_label, "⚡ --");
    lv_obj_set_style_text_font(ls.battery_label,
        &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(ls.battery_label, COLOR_GOLD, 0);
    lv_obj_align(ls.battery_label, LV_ALIGN_BOTTOM_LEFT, 15, -15);

    /* Hint de desbloqueio (parte inferior) */
    ls.unlock_hint = lv_label_create(ls.root);
    lv_label_set_text(ls.unlock_hint, "▽ Toque para desbloquear");
    lv_obj_set_style_text_font(ls.unlock_hint,
        &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(ls.unlock_hint, COLOR_TEXT_DIM, 0);
    lv_obj_align(ls.unlock_hint, LV_ALIGN_BOTTOM_MID, 0, -20);

    /* Eventos */
    lv_obj_add_event_cb(ls.root, screen_event_cb,
        LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(ls.root, screen_event_cb,
        LV_EVENT_SCROLL_BEGIN, NULL);

    /* Timer de atualização (1 Hz) */
    ls.timer = lv_timer_create(update_time_cb, 1000, NULL);
    update_time_cb(NULL);   /* Primeira atualização imediata */
}

void runesos_lockscreen_destroy(void)
{
    if (ls.timer) {
        lv_timer_delete(ls.timer);
        ls.timer = NULL;
    }
    if (ls.root) {
        lv_obj_delete(ls.root);
        ls.root = NULL;
    }
    ls.active = false;
}

bool runesos_lockscreen_is_active(void)
{
    return ls.active;
}

void runesos_lockscreen_lock(lv_obj_t *parent,
                              const runesos_hal_t *hal,
                              runesos_unlock_cb_t on_unlock)
{
    runesos_lockscreen_destroy();
    runesos_lockscreen_create(parent, hal, on_unlock);
}