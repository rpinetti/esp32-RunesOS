#ifndef RUNESOS_APP_H
#define RUNESOS_APP_H

#include "lvgl.h"

/* Contrato que todo app do RunesOS deve implementar */
typedef struct {
    const char *name;
    const void *icon;           /* ponteiro para o ícone (imagem) */
    const char *rune;           /* Símbolo rúnico (ícone) */
    lv_color_t  icon_color;     /* Cor do ícone */
    void (*create)(lv_obj_t *parent);  /* cria a tela do app */
    void (*destroy)(void);
} runesos_app_t;

/* Registry — array global de apps registrados */
const runesos_app_t *runesos_app_registry_get(uint8_t *count);

/* Busca app por índice */
const runesos_app_t *runesos_app_get(uint8_t index);

#endif