#include "runesos_app.h"

/* ---- Forward declarations dos apps reais ---- */
void app_oraculo_create(lv_obj_t *parent);
void app_oraculo_destroy(void);

void app_relogio_create(lv_obj_t *parent);
void app_relogio_destroy(void);

void app_notas_create(lv_obj_t *parent);
void app_notas_destroy(void);

void app_terminal_create(lv_obj_t *parent);
void app_terminal_destroy(void);

void app_config_create(lv_obj_t *parent);
void app_config_destroy(void);

void app_files_create(lv_obj_t *parent);
void app_files_destroy(void);

void app_tarefas_create(lv_obj_t *parent);
void app_tarefas_destroy(void);

void app_tempo_create(lv_obj_t *parent);
void app_tempo_destroy(void);

void app_media_create(lv_obj_t *parent);
void app_media_destroy(void);

/* ---- Placeholders ---- */
void app_tinyml_create(lv_obj_t *parent);
void app_tinyml_destroy(void);

void app_jogos_create(lv_obj_t *parent);
void app_jogos_destroy(void);

/* Cores do tema nórdico (LVGL 9 struct initialization) */
#define MAKE_COLOR(r, g, b) ((lv_color_t){ .blue = (b), .green = (g), .red = (r) })
#define C_GOLD  MAKE_COLOR(0xD4, 0xAF, 0x37)
#define C_BLUE  MAKE_COLOR(0x4A, 0x6F, 0xA5)
#define C_GREEN MAKE_COLOR(0x6B, 0x8E, 0x6B)
#define C_RED   MAKE_COLOR(0xB8, 0x54, 0x50)
#define C_PURP  MAKE_COLOR(0x8B, 0x5C, 0xF6)
#define C_TEAL  MAKE_COLOR(0x2A, 0x9D, 0x8F)
#define C_SLATE MAKE_COLOR(0x4E, 0x5D, 0x6C)
#define C_RUST  MAKE_COLOR(0xC8, 0x64, 0x46)

/* Registry estático de aplicativos */
static const runesos_app_t apps[] = {
    {
        .name = "Oráculo",
        .rune = "ᚱ",  /* Raidho */
        .icon_color = C_GOLD,
        .create = app_oraculo_create,
        .destroy = app_oraculo_destroy,
    },
    {
        .name = "Relógio",
        .rune = "ᛏ",  /* Teiwaz */
        .icon_color = C_BLUE,
        .create = app_relogio_create,
        .destroy = app_relogio_destroy,
    },
    {
        .name = "Notas",
        .rune = "ᚾ",  /* Nauthiz */
        .icon_color = C_GREEN,
        .create = app_notas_create,
        .destroy = app_notas_destroy,
    },
    {
        .name = "Terminal",
        .rune = "ᛁ",  /* Isa */
        .icon_color = C_TEAL,
        .create = app_terminal_create,
        .destroy = app_terminal_destroy,
    },
    {
        .name = "Arquivos",
        .rune = "ᛋ",  /* Sowilo */
        .icon_color = C_GOLD,
        .create = app_files_create,
        .destroy = app_files_destroy,
    },
    {
        .name = "Tarefas",
        .rune = "ᚦ",  /* Thurisaz */
        .icon_color = C_RUST,
        .create = app_tarefas_create,
        .destroy = app_tarefas_destroy,
    },
    {
        .name = "Ajustes",
        .rune = "ᛟ",  /* Othala */
        .icon_color = C_SLATE,
        .create = app_config_create,
        .destroy = app_config_destroy,
    },
    {
        .name = "Tempo",
        .rune = "ᛜ",  /* Ingwaz */
        .icon_color = C_BLUE,
        .create = app_tempo_create,
        .destroy = app_tempo_destroy,
    },
    {
        .name = "Mídia",
        .rune = "ᛗ",  /* Mannaz */
        .icon_color = C_PURP,
        .create = app_media_create,
        .destroy = app_media_destroy,
    },
    {
        .name = "TinyML",
        .rune = "ᛒ",  /* Berkano */
        .icon_color = C_RED,
        .create = app_tinyml_create,
        .destroy = app_tinyml_destroy,
    },
    {
        .name = "Jogos",
        .rune = "ᛚ",  /* Laguz */
        .icon_color = C_PURP,
        .create = app_jogos_create,
        .destroy = app_jogos_destroy,
    },
};

const runesos_app_t *runesos_app_registry_get(uint8_t *count)
{
    *count = sizeof(apps) / sizeof(apps[0]);
    return apps;
}

const runesos_app_t *runesos_app_get(uint8_t index)
{
    if (index >= sizeof(apps) / sizeof(apps[0]))
        return NULL;
    return &apps[index];
}
