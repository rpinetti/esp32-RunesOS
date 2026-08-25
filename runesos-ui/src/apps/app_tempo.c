#include "runesos_app.h"
#include "theme_nordico.h"
#include <stdio.h>

static lv_obj_t *s_city_lbl = NULL;
static lv_obj_t *s_temp_lbl = NULL;
static lv_obj_t *s_stat_lbl = NULL;

typedef struct {
    const char *day;
    const char *icon;
    const char *condition;
    int8_t      max_t;
    int8_t      min_t;
} forecast_day_t;

static const forecast_day_t s_forecast[] = {
    {"Hoje", "🌤️", "Parcialmente Nublado", 15,  8},
    {"Amanhã", "🌧️", "Chuva Moderada",     12,  6},
    {"Quarta", "⛅", "Nublado",              13,  7},
    {"Quinta", "☀️", "Ensolarado",           17,  9},
    {"Sexta",  "❄️", "Geada e Neve",         4,  -2}
};

#define FORECAST_DAYS (sizeof(s_forecast)/sizeof(s_forecast[0]))

static void refresh_btn_cb(lv_event_t *e)
{
    (void)e;
    if (s_stat_lbl) {
        lv_label_set_text(s_stat_lbl, "🔄 Dados meteorológicos sincronizados!");
        lv_obj_set_style_text_color(s_stat_lbl, RUNESOS_COLOR_ACCENT, 0);
    }
}

void app_tempo_create(lv_obj_t *parent)
{
    lv_obj_set_style_pad_all(parent, 10, 0);
    lv_obj_set_style_pad_row(parent, 12, 0);
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);

    /* 1. Hero Card: Cidade e Temperatura Atual */
    lv_obj_t *hero = lv_obj_create(parent);
    lv_obj_set_size(hero, LV_PCT(100), 140);
    lv_obj_set_style_bg_color(hero, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(hero, 16, 0);
    lv_obj_set_style_border_width(hero, 1, 0);
    lv_obj_set_style_border_color(hero, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_scrollable(hero, false);

    s_city_lbl = lv_label_create(hero);
    lv_label_set_text(s_city_lbl, "📍 Oslo, Noruega");
    lv_obj_set_style_text_font(s_city_lbl, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(s_city_lbl, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_align(s_city_lbl, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_obj_t *icon_lbl = lv_label_create(hero);
    lv_label_set_text(icon_lbl, "🌤️");
    lv_obj_set_style_text_font(icon_lbl, &lv_font_montserrat_36, 0);
    lv_obj_align(icon_lbl, LV_ALIGN_LEFT_MID, 10, 10);

    s_temp_lbl = lv_label_create(hero);
    lv_label_set_text(s_temp_lbl, "14°C");
    lv_obj_set_style_text_font(s_temp_lbl, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(s_temp_lbl, RUNESOS_COLOR_TEXT, 0);
    lv_obj_align(s_temp_lbl, LV_ALIGN_RIGHT_MID, -10, 0);

    lv_obj_t *cond_lbl = lv_label_create(hero);
    lv_label_set_text(cond_lbl, "Parcialmente Nublado  •  Runa ᛜ Ingwaz");
    lv_obj_set_style_text_font(cond_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(cond_lbl, lv_color_hex(0x8888AA), 0);
    lv_obj_align(cond_lbl, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    /* 2. Grid de Métricas (2x2) */
    lv_obj_t *m_grid = lv_obj_create(parent);
    lv_obj_set_size(m_grid, LV_PCT(100), 80);
    lv_obj_set_style_bg_opa(m_grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(m_grid, 0, 0);
    lv_obj_set_style_pad_all(m_grid, 0, 0);
    lv_obj_set_scrollable(m_grid, false);
    lv_obj_set_flex_flow(m_grid, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(m_grid, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    /* Umidade */
    lv_obj_t *c_umid = lv_obj_create(m_grid);
    lv_obj_set_size(c_umid, 105, 75);
    lv_obj_set_style_bg_color(c_umid, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c_umid, 10, 0);
    lv_obj_set_style_border_width(c_umid, 1, 0);
    lv_obj_set_style_border_color(c_umid, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c_umid, false);
    lv_obj_t *lu1 = lv_label_create(c_umid);
    lv_label_set_text(lu1, "💧 Umidade\n68%");
    lv_obj_set_style_text_align(lu1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lu1);

    /* Vento */
    lv_obj_t *c_vent = lv_obj_create(m_grid);
    lv_obj_set_size(c_vent, 105, 75);
    lv_obj_set_style_bg_color(c_vent, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c_vent, 10, 0);
    lv_obj_set_style_border_width(c_vent, 1, 0);
    lv_obj_set_style_border_color(c_vent, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c_vent, false);
    lv_obj_t *lu2 = lv_label_create(c_vent);
    lv_label_set_text(lu2, "💨 Vento\n18 km/h");
    lv_obj_set_style_text_align(lu2, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lu2);

    /* Sensação */
    lv_obj_t *c_sens = lv_obj_create(m_grid);
    lv_obj_set_size(c_sens, 105, 75);
    lv_obj_set_style_bg_color(c_sens, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c_sens, 10, 0);
    lv_obj_set_style_border_width(c_sens, 1, 0);
    lv_obj_set_style_border_color(c_sens, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c_sens, false);
    lv_obj_t *lu3 = lv_label_create(c_sens);
    lv_label_set_text(lu3, "🌡️ Sensação\n12°C");
    lv_obj_set_style_text_align(lu3, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lu3);

    /* Pressão */
    lv_obj_t *c_pres = lv_obj_create(m_grid);
    lv_obj_set_size(c_pres, 105, 75);
    lv_obj_set_style_bg_color(c_pres, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(c_pres, 10, 0);
    lv_obj_set_style_border_width(c_pres, 1, 0);
    lv_obj_set_style_border_color(c_pres, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_scrollable(c_pres, false);
    lv_obj_t *lu4 = lv_label_create(c_pres);
    lv_label_set_text(lu4, "📊 Pressão\n1014 hPa");
    lv_obj_set_style_text_align(lu4, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(lu4);

    /* 3. Previsão da Semana */
    lv_obj_t *fc_box = lv_obj_create(parent);
    lv_obj_set_size(fc_box, LV_PCT(100), 200);
    lv_obj_set_style_bg_color(fc_box, RUNESOS_COLOR_SURFACE, 0);
    lv_obj_set_style_radius(fc_box, 12, 0);
    lv_obj_set_style_border_width(fc_box, 1, 0);
    lv_obj_set_style_border_color(fc_box, lv_color_hex(0x2A3B5C), 0);
    lv_obj_set_style_pad_all(fc_box, 8, 0);
    lv_obj_set_style_pad_row(fc_box, 6, 0);
    lv_obj_set_flex_flow(fc_box, LV_FLEX_FLOW_COLUMN);

    for (size_t i = 0; i < FORECAST_DAYS; i++) {
        lv_obj_t *row = lv_obj_create(fc_box);
        lv_obj_set_size(row, LV_PCT(100), 32);
        lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_pad_all(row, 0, 0);
        lv_obj_set_scrollable(row, false);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                              LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        /* Dia + Ícone */
        lv_obj_t *dl = lv_label_create(row);
        char d_str[32];
        snprintf(d_str, sizeof(d_str), "%s %s", s_forecast[i].day, s_forecast[i].icon);
        lv_label_set_text(dl, d_str);
        lv_obj_set_style_text_color(dl, RUNESOS_COLOR_TEXT, 0);

        /* Condição */
        lv_obj_t *cl = lv_label_create(row);
        lv_label_set_text(cl, s_forecast[i].condition);
        lv_obj_set_style_text_font(cl, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(cl, lv_color_hex(0x8888AA), 0);

        /* Temp Max / Min */
        lv_obj_t *tl = lv_label_create(row);
        char t_str[16];
        snprintf(t_str, sizeof(t_str), "%d° / %d°C", s_forecast[i].max_t, s_forecast[i].min_t);
        lv_label_set_text(tl, t_str);
        lv_obj_set_style_text_color(tl, RUNESOS_COLOR_ACCENT, 0);
    }

    /* 4. Botão Atualizar */
    lv_obj_t *ref_btn = lv_btn_create(parent);
    lv_obj_set_size(ref_btn, LV_PCT(100), 44);
    lv_obj_set_style_bg_color(ref_btn, RUNESOS_COLOR_ACCENT, 0);
    lv_obj_set_style_radius(ref_btn, 10, 0);
    lv_obj_t *rl = lv_label_create(ref_btn);
    lv_label_set_text(rl, "🔄 Atualizar Previsão");
    lv_obj_set_style_text_color(rl, lv_color_hex(0x1A1A2E), 0);
    lv_obj_set_style_text_font(rl, &lv_font_montserrat_14, 0);
    lv_obj_center(rl);
    lv_obj_add_event_cb(ref_btn, refresh_btn_cb, LV_EVENT_CLICKED, NULL);

    s_stat_lbl = lv_label_create(parent);
    lv_label_set_text(s_stat_lbl, "");
    lv_obj_set_style_text_align(s_stat_lbl, LV_TEXT_ALIGN_CENTER, 0);
}

void app_tempo_destroy(void)
{
    s_city_lbl = NULL;
    s_temp_lbl = NULL;
    s_stat_lbl = NULL;
}

