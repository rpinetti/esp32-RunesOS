#ifndef RUNESOS_FW_H
#define RUNESOS_FW_H

#include "hal/hal_runesos.h"

/* Registra a implementação do HAL ativa (mock no PC, board na placa).
 * A porta chama isto ANTES de runesos_fw_init(). */
void runesos_fw_set_hal(const runesos_hal_t *hal);

/* Retorna o HAL ativo — usado pelos services e pela UI. */
const runesos_hal_t *runesos_fw_get_hal(void);

/* Inicializa o firmware: valida o HAL e inicializa os services. */
void runesos_fw_init(void);

#endif