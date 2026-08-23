#ifndef RUNESOS_THEME_H
#define RUNESOS_THEME_H

#include "lvgl.h"

/* Paleta nórdica */
#define RUNESOS_COLOR_BG       lv_color_hex(0x1a1a2e)  /* azul-noite */
#define RUNESOS_COLOR_SURFACE  lv_color_hex(0x16213e)  /* azul profundo */
#define RUNESOS_COLOR_ACCENT   lv_color_hex(0xd4af37)  /* dourado runas */
#define RUNESOS_COLOR_ICE      lv_color_hex(0x0f3460)  /* azul gelo */
#define RUNESOS_COLOR_TEXT     lv_color_hex(0xeeeeee)  /* branco gelo */

void runesos_theme_apply(lv_obj_t *scr);

#endif