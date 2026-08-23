#ifndef RUNESOS_APP_H
#define RUNESOS_APP_H

#include "lvgl.h"

typedef struct {
    const char *name;
    const void *icon;          /* ponteiro para o ícone (imagem) */
    lv_obj_t *(*create)(void); /* cria a tela do app */
    void (*destroy)(lv_obj_t *scr);
} runesos_app_t;

#endif