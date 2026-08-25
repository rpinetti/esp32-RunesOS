#include "runesos_session.h"
#include "runesos_lockscreen.h"
#include "runesos_homescreen.h"
#include "runesos_app.h"
#include "theme_nordico.h"
#include <stdio.h>

static struct {
    lv_obj_t           *root;
    const runesos_hal_t *hal;
    lv_obj_t           *app_container;   /* Container do app ativo */
    const runesos_app_t *current_app;   /* App aberto (NULL = home) */
} sess;

static void on_unlock(void)
{
    /* Lock screen destruído pela própria animação.
     * Home screen já está visível embaixo. */
}

static void on_app_launch(uint8_t index)
{
    runesos_session_open_app(index);
}

void runesos_session_init(lv_obj_t *root, const runesos_hal_t *hal)
{
    sess.root = root;
    sess.hal = hal;
    sess.current_app = NULL;

    /* 1. Cria a home screen (camada base) */
    runesos_homescreen_create(root, hal, on_app_launch);

    /* 2. Cria o lock screen por cima */
    runesos_lockscreen_create(root, hal, on_unlock);
}

void runesos_session_open_app(uint8_t app_index)
{
    const runesos_app_t *app = runesos_app_get(app_index);
    if (!app || sess.current_app)
        return;

    sess.current_app = app;

    /* Container fullscreen para o app */
    sess.app_container = lv_obj_create(sess.root);
    lv_obj_set_size(sess.app_container, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(sess.app_container, 0, 0);
    lv_obj_set_scrollable(sess.app_container, false);
    lv_obj_set_style_bg_color(sess.app_container, RUNESOS_COLOR_BG, 0);
    lv_obj_set_style_border_width(sess.app_container, 0, 0);
    lv_obj_set_style_pad_all(sess.app_container, 0, 0);

    /* Header superior do App */
    lv_obj_t *header = lv_obj_create(sess.app_container);
    lv_obj_set_size(header, LV_PCT(100), 46);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_hor(header, 10, 0);
    lv_obj_set_style_pad_ver(header, 4, 0);
    lv_obj_set_scrollable(header, false);

    /* Botão Voltar (ᛒ) */
    lv_obj_t *back_btn = lv_btn_create(header);
    lv_obj_set_size(back_btn, 60, 36);
    lv_obj_align(back_btn, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_bg_color(back_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(back_btn, 8, 0);
    lv_obj_set_style_pad_all(back_btn, 0, 0);

    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "ᛒ Voltar");  /* Berkano = Voltar */
    lv_obj_set_style_text_font(back_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(back_label, lv_color_hex(0x1A1A2E), 0);
    lv_obj_center(back_label);

    /* Título Central com a Runa e Nome do App */
    lv_obj_t *title_label = lv_label_create(header);
    char title_buf[64];
    snprintf(title_buf, sizeof(title_buf), "%s  %s",
             app->rune ? app->rune : "",
             app->name ? app->name : "");
    lv_label_set_text(title_label, title_buf);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(title_label, RUNESOS_COLOR_TEXT, 0);
    lv_obj_center(title_label);

    /* Container de conteúdo do app (abaixo do header) */
    lv_obj_t *content = lv_obj_create(sess.app_container);
    lv_obj_set_size(content, 480, 594);
    lv_obj_align(content, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_style_pad_all(content, 12, 0);

    /* Chama a função create do app */
    app->create(content);

    /* Evento do botão voltar */
    lv_obj_add_event_cb(back_btn, (lv_event_cb_t)runesos_session_close_app,
        LV_EVENT_CLICKED, NULL);
}

void runesos_session_close_app(void)
{
    if (!sess.current_app)
        return;

    /* Destrói o app */
    if (sess.current_app->destroy) {
        sess.current_app->destroy();
    }

    /* Remove o container do app */
    if (sess.app_container) {
        lv_obj_delete(sess.app_container);
        sess.app_container = NULL;
    }

    sess.current_app = NULL;
}

void runesos_session_lock(void)
{
    if (runesos_lockscreen_is_active())
        return;
    runesos_lockscreen_lock(sess.root, sess.hal, on_unlock);
}