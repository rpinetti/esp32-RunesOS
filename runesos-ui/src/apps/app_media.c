#include "runesos_app.h"
#include "theme_nordico.h"
#include "hal_runesos.h"
#include <stdio.h>
#include <stdlib.h>

extern const runesos_hal_t runesos_hal_mock;

#define EQ_BARS 10

static lv_obj_t   *s_play_btn_lbl = NULL;
static lv_obj_t   *s_time_lbl = NULL;
static lv_obj_t   *s_slider = NULL;
static lv_obj_t   *s_eq_bars[EQ_BARS];
static lv_timer_t *s_media_timer = NULL;
static bool        s_is_playing = false;
static uint32_t    s_current_sec = 102; /* 01:42 */
static uint32_t    s_total_sec = 234;   /* 03:54 */

typedef struct {
    const char *title;
    const char *artist;
    uint32_t    duration_sec;
} track_t;

static const track_t s_playlist[] = {
    {"Hymn of the North Winds", "Wardruna • Runaljod", 234},
    {"Raidho - Journey of Kings", "Danheim • Dombap", 195},
    {"Valhalla Calling", "Miracle of Sound • Folk", 212},
};

static size_t s_current_track_idx = 0;
static lv_obj_t *s_track_title_lbl = NULL;
static lv_obj_t *s_track_artist_lbl = NULL;

static void update_track_info(void)
{
    if (s_track_title_lbl) {
        lv_label_set_text(s_track_title_lbl, s_playlist[s_current_track_idx].title);
    }
    if (s_track_artist_lbl) {
        lv_label_set_text(s_track_artist_lbl, s_playlist[s_current_track_idx].artist);
    }
    s_total_sec = s_playlist[s_current_track_idx].duration_sec;
    s_current_sec = 0;
    if (s_slider) {
        lv_slider_set_range(s_slider, 0, s_total_sec);
        lv_slider_set_value(s_slider, 0, LV_ANIM_OFF);
    }
}

static void media_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (!s_is_playing) return;

    /* Avança tempo */
    s_current_sec++;
    if (s_current_sec > s_total_sec) {
        s_current_sec = 0;
    }

    if (s_slider) {
        lv_slider_set_value(s_slider, s_current_sec, LV_ANIM_OFF);
    }

    if (s_time_lbl) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%02u:%02u / %02u:%02u",
                 s_current_sec / 60, s_current_sec % 60,
                 s_total_sec / 60, s_total_sec % 60);
        lv_label_set_text(s_time_lbl, buf);
    }

    /* Anima barras do equalizador */
    for (int i = 0; i < EQ_BARS; i++) {
        if (s_eq_bars[i]) {
            int h = 6 + (rand() % 38);
            lv_obj_set_height(s_eq_bars[i], h);
        }
    }
}

static void play_toggle_cb(lv_event_t *e)
{
    (void)e;
    s_is_playing = !s_is_playing;
    if (s_play_btn_lbl) {
        lv_label_set_text(s_play_btn_lbl, s_is_playing ? "⏸️" : "▶️");
    }
    if (!s_is_playing) {
        for (int i = 0; i < EQ_BARS; i++) {
            if (s_eq_bars[i]) {
                lv_obj_set_height(s_eq_bars[i], 4);
            }
        }
    }
}

static void next_track_cb(lv_event_t *e)
{
    (void)e;
    s_current_track_idx = (s_current_track_idx + 1) % (sizeof(s_playlist)/sizeof(s_playlist[0]));
    update_track_info();
}

static void prev_track_cb(lv_event_t *e)
{
    (void)e;
    if (s_current_track_idx == 0) {
        s_current_track_idx = (sizeof(s_playlist)/sizeof(s_playlist[0])) - 1;
    } else {
        s_current_track_idx--;
    }
    update_track_info();
}

void app_media_create(lv_obj_t *parent)
{
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_row(parent, 14, 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scrollable(parent, false);

    /* 1. Capa Rúnica do Álbum */
    lv_obj_t *cover = lv_obj_create(parent);
    lv_obj_set_size(cover, 160, 160);
    lv_obj_align(cover, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(cover, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(cover, 20, 0);
    lv_obj_set_style_border_width(cover, 2, 0);
    lv_obj_set_style_border_color(cover, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_scrollable(cover, false);

    lv_obj_t *rune_art = lv_label_create(cover);
    lv_label_set_text(rune_art, "ᛗ");
    lv_obj_set_style_text_font(rune_art, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(rune_art, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_center(rune_art);

    /* 2. Título e Artista */
    s_track_title_lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(s_track_title_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_track_title_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_set_style_text_align(s_track_title_lbl, LV_TEXT_ALIGN_CENTER, 0);

    s_track_artist_lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(s_track_artist_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_track_artist_lbl, lv_color_hex(0x8888AA), 0);
    lv_obj_set_style_text_align(s_track_artist_lbl, LV_TEXT_ALIGN_CENTER, 0);

    update_track_info();

    /* 3. Equalizador Visual (Barras animadas) */
    lv_obj_t *eq_box = lv_obj_create(parent);
    lv_obj_set_size(eq_box, 320, 50);
    lv_obj_set_style_bg_opa(eq_box, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(eq_box, 0, 0);
    lv_obj_set_style_pad_all(eq_box, 0, 0);
    lv_obj_set_scrollable(eq_box, false);
    lv_obj_set_flex_flow(eq_box, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(eq_box, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);

    for (int i = 0; i < EQ_BARS; i++) {
        s_eq_bars[i] = lv_obj_create(eq_box);
        lv_obj_set_size(s_eq_bars[i], 16, 6);
        lv_obj_set_style_bg_color(s_eq_bars[i], RUNESOS_COLOR_ACCENT, 0);
        lv_obj_set_style_radius(s_eq_bars[i], 4, 0);
        lv_obj_set_style_border_width(s_eq_bars[i], 0, 0);
        lv_obj_set_scrollable(s_eq_bars[i], false);
    }

    /* 4. Barra de Progresso e Tempo */
    s_slider = lv_slider_create(parent);
    lv_obj_set_size(s_slider, 380, 10);
    lv_slider_set_range(s_slider, 0, s_total_sec);
    lv_slider_set_value(s_slider, s_current_sec, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(s_slider, RUNESOS_COLOR_ACCENT, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(s_slider, RUNESOS_COLOR_ACCENT, LV_PART_KNOB);

    s_time_lbl = lv_label_create(parent);
    lv_label_set_text(s_time_lbl, "01:42 / 03:54");
    lv_obj_set_style_text_font(s_time_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(s_time_lbl, lv_color_hex(0x8888AA), 0);
    lv_obj_set_style_text_align(s_time_lbl, LV_TEXT_ALIGN_CENTER, 0);

    /* 5. Controles de Playback */
    lv_obj_t *ctrl_row = lv_obj_create(parent);
    lv_obj_set_size(ctrl_row, 300, 60);
    lv_obj_set_style_bg_opa(ctrl_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_row, 0, 0);
    lv_obj_set_style_pad_all(ctrl_row, 0, 0);
    lv_obj_set_scrollable(ctrl_row, false);
    lv_obj_set_flex_flow(ctrl_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_row, LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Botão Anterior */
    lv_obj_t *prev_btn = lv_btn_create(ctrl_row);
    lv_obj_set_size(prev_btn, 60, 50);
    lv_obj_set_style_bg_color(prev_btn, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(prev_btn, 12, 0);
    lv_obj_t *p_lbl = lv_label_create(prev_btn);
    lv_label_set_text(p_lbl, "⏮️");
    lv_obj_center(p_lbl);
    lv_obj_add_event_cb(prev_btn, prev_track_cb, LV_EVENT_CLICKED, NULL);

    /* Botão Play/Pause */
    lv_obj_t *play_btn = lv_btn_create(ctrl_row);
    lv_obj_set_size(play_btn, 70, 55);
    lv_obj_set_style_bg_color(play_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(play_btn, 16, 0);
    s_play_btn_lbl = lv_label_create(play_btn);
    lv_label_set_text(s_play_btn_lbl, "▶️");
    lv_obj_set_style_text_font(s_play_btn_lbl, &lv_font_montserrat_16, 0);
    lv_obj_center(s_play_btn_lbl);
    lv_obj_add_event_cb(play_btn, play_toggle_cb, LV_EVENT_CLICKED, NULL);

    /* Botão Próxima */
    lv_obj_t *next_btn = lv_btn_create(ctrl_row);
    lv_obj_set_size(next_btn, 60, 50);
    lv_obj_set_style_bg_color(next_btn, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(next_btn, 12, 0);
    lv_obj_t *n_lbl = lv_label_create(next_btn);
    lv_label_set_text(n_lbl, "⏭️");
    lv_obj_center(n_lbl);
    lv_obj_add_event_cb(next_btn, next_track_cb, LV_EVENT_CLICKED, NULL);

    s_media_timer = lv_timer_create(media_timer_cb, 200, NULL);
}

void app_media_destroy(void)
{
    if (s_media_timer) {
        lv_timer_delete(s_media_timer);
        s_media_timer = NULL;
    }
    s_play_btn_lbl = NULL;
    s_time_lbl = NULL;
    s_slider = NULL;
    s_track_title_lbl = NULL;
    s_track_artist_lbl = NULL;
    for (int i = 0; i < EQ_BARS; i++) {
        s_eq_bars[i] = NULL;
    }
}

