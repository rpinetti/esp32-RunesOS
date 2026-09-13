#ifndef RUNESOS_LOCKSCREEN_H
#define RUNESOS_LOCKSCREEN_H

#include "lvgl.h"
#include "lvgl_compat.h"
#include "hal_runesos.h"


typedef void (*runesos_unlock_cb_t)(void);

/**
 * Cria a tela de bloqueio em fullscreen sobre o parent.
 * @param parent     Screen/container pai
 * @param hal        HAL para obter tempo e bateria
 * @param on_unlock  Callback chamado ao desbloquear
 */
void runesos_lockscreen_create(lv_obj_t *parent,
                                const runesos_hal_t *hal,
                                runesos_unlock_cb_t on_unlock);

/** Destrói a tela de bloqueio (após animação de unlock). */
void runesos_lockscreen_destroy(void);

/** Retorna true se o lock screen está visível. */
bool runesos_lockscreen_is_active(void);

/** Força o bloqueio (recria o lock screen). */
void runesos_lockscreen_lock(lv_obj_t *parent,
                              const runesos_hal_t *hal,
                              runesos_unlock_cb_t on_unlock);

#endif // RUNESOS_LOCKSCREEN_H