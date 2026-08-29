#ifndef RUNESOS_THEME_H
#define RUNESOS_THEME_H

#include "lvgl.h"

/* Paleta nórdica */
#define RUNESOS_COLOR_BG       lv_color_hex(0x0d1b2a)  /* azul fiorde */
#define RUNESOS_COLOR_SURFACE  lv_color_hex(0x495057)  /* cinza forja */
#define RUNESOS_COLOR_ACCENT   lv_color_hex(0xd4af37)  /* dourado runas */
#define RUNESOS_COLOR_ICE      lv_color_hex(0x66fcf1)  /* ciano glacial */
#define RUNESOS_COLOR_TEXT     lv_color_hex(0xf8f9fa)  /* branco runico */

void runesos_theme_apply(lv_obj_t *scr);

#endif