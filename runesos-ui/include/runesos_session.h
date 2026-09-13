#ifndef RUNESOS_SESSION_H
#define RUNESOS_SESSION_H

#include "lvgl.h"
#include "lvgl_compat.h"
#include "hal_runesos.h"


/**
 * Inicializa a sessão do RunesOS.
 * Cria a home screen e exibe o lock screen por cima.
 * Deve ser chamado uma única vez no main().
 */
void runesos_session_init(lv_obj_t *root, const runesos_hal_t *hal);

/** Abre um app pelo índice no registry. */
void runesos_session_open_app(uint8_t app_index);

/** Fecha o app atual e volta para a home screen. */
void runesos_session_close_app(void);

/** Bloqueia o dispositivo (volta para o lock screen). */
void runesos_session_lock(void);

#endif // RUNESOS_SESSION_H