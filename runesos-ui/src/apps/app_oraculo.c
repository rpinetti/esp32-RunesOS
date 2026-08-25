#include "runesos_app.h"
#include <stdlib.h>
#include <time.h>

/* Elder Futhark — 24 runas */
typedef struct {
    const char *rune;
    const char *name;
    const char *meaning;
} rune_entry_t;

static const rune_entry_t elder_futhark[] = {
    {"ᚠ", "Fehu",    "Riqueza, prosperidade, abundância"},
    {"ᚢ", "Uruz",   "Força bruta, vitalidade, coragem"},
    {"ᚦ", "Thurisaz","Defesa, conflito, espinho protetor"},
    {"ᚨ", "Ansuz",  "Comunicação, sabedoria, mensagens divinas"},
    {"ᚱ", "Raidho", "Jornada, movimento, ritmo da vida"},
    {"ᚲ", "Kenaz",  "Fogo criativo, iluminação, conhecimento"},
    {"ᚷ", "Gebo",   "Presente, troca, equilíbrio mútuo"},
    {"ᚹ", "Wunjo",  "Alegria, harmonia, realização"},
    {"ᚺ", "Hagalaz","Tempestade, destruição transformadora"},
    {"ᚾ", "Nauthiz","Necessidade, resistência, paciência"},
    {"ᛁ", "Isa",    "Gelo, pausa, introspecção"},
    {"ᛃ", "Jera",   "Colheita, ciclos, recompensa justa"},
    {"ᛇ", "Eihwaz", "Eixo do mundo, resistência, transformação"},
    {"ᛈ", "Perthro","Mistério, destino, runas ocultas"},
    {"ᛉ", "Algiz",  "Proteção, conexão divina, escudo"},
    {"ᛋ", "Sowilo", "Sol, sucesso, vitória, energia vital"},
    {"ᛏ", "Tiwaz",  "Justiça, sacrifício, honra"},
    {"ᛒ", "Berkano","Renascimento, crescimento, feminino"},
    {"ᛖ", "Ehwaz",  "Movimento, parceria, confiança"},
    {"ᛗ", "Mannaz", "Humanidade, cooperação, eu"},
    {"ᛚ", "Laguz",  "Água, fluxo, intuição, inconsciente"},
    {"ᛜ", "Ingwaz", "Fertilidade, novo começo, potencial"},
    {"ᛟ", "Othala", "Herança, lar, ancestralidade"},
    {"ᛞ", "Dagaz",  "Amanhecer, despertar, clareza"},
};

#define RUNE_COUNT (sizeof(elder_futhark) / sizeof(elder_futhark[0]))

static lv_obj_t *rune_label;
static lv_obj_t *name_label;
static lv_obj_t *meaning_label;
static lv_obj_t *draw_btn;

static void draw_rune(lv_obj_t *btn)
{
    (void)btn;
    uint8_t idx = rand() % RUNE_COUNT;
    const rune_entry_t *r = &elder_futhark[idx];

    lv_label_set_text(rune_label, r->rune);
    lv_label_set_text(name_label, r->name);
    lv_label_set_text(meaning_label, r->meaning);
}

void app_oraculo_create(lv_obj_t *parent)
{
    srand(time(NULL));

    /* Título */
    lv_obj_t *title = lv_label_create(parent);
    lv_label_set_text(title, "ᚱ Oráculo Rúnico");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xD4AF37), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* Runa sorteada (grande) */
    rune_label = lv_label_create(parent);
    lv_label_set_text(rune_label, "ᚱ");
    lv_obj_set_style_text_font(rune_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(rune_label, lv_color_hex(0xE0E0E0), 0);
    lv_obj_align(rune_label, LV_ALIGN_CENTER, 0, -40);

    /* Nome da runa */
    name_label = lv_label_create(parent);
    lv_label_set_text(name_label, "Raidho");
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xD4AF37), 0);
    lv_obj_align(name_label, LV_ALIGN_CENTER, 0, 15);

    /* Significado */
    meaning_label = lv_label_create(parent);
    lv_label_set_text(meaning_label, "Jornada, movimento, ritmo da vida");
    lv_obj_set_style_text_font(meaning_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(meaning_label, lv_color_hex(0x8888AA), 0);
    lv_obj_set_width(meaning_label, 200);
    lv_label_set_long_mode(meaning_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(meaning_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(meaning_label, LV_ALIGN_CENTER, 0, 50);

    /* Botão "Sortear Runa" */
    draw_btn = lv_btn_create(parent);
    lv_obj_set_size(draw_btn, 160, 45);
    lv_obj_align(draw_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(draw_btn, lv_color_hex(0xD4AF37), 0);
    lv_obj_set_style_radius(draw_btn, 10, 0);

    lv_obj_t *btn_label = lv_label_create(draw_btn);
    lv_label_set_text(btn_label, "Sortear Runa");
    lv_obj_set_style_text_color(btn_label, lv_color_hex(0x1A1A2E), 0);
    lv_obj_center(btn_label);

    lv_obj_add_event_cb(draw_btn, (lv_event_cb_t)draw_rune,
        LV_EVENT_CLICKED, NULL);
}

void app_oraculo_destroy(void)
{
    /* LVGL deleta os objetos quando o parent é destruído.
     * Apenas resetamos os ponteiros. */
    rune_label = NULL;
    name_label = NULL;
    meaning_label = NULL;
    draw_btn = NULL;
}