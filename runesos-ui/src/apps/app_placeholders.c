#include "lvgl.h"

#define DEFINE_PLACEHOLDER(name, title, rune)                    \
    static lv_obj_t *ph_label_##name;                            \
    void app_##name##_create(lv_obj_t *parent)                   \
    {                                                            \
        ph_label_##name = lv_label_create(parent);               \
        lv_label_set_text(ph_label_##name,                       \
            rune " " title "\n\n[ Em desenvolvimento ]");       \
        lv_obj_set_style_text_font(ph_label_##name,             \
            &lv_font_montserrat_20, 0);                          \
        lv_obj_set_style_text_color(ph_label_##name,            \
            lv_color_hex(0xD4AF37), 0);                          \
        lv_obj_align(ph_label_##name, LV_ALIGN_CENTER, 0, 0);   \
    }                                                            \
    void app_##name##_destroy(void)                              \
    {                                                            \
        ph_label_##name = NULL;                                  \
    }

DEFINE_PLACEHOLDER(tinyml,   "TinyML",             "ᛒ")
DEFINE_PLACEHOLDER(jogos,    "Jogos Game Boy",     "ᛚ")

