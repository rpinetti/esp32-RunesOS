#include "runesos_fw.h"
#include "battery_service.h"
#include "radio_service.h"
#include "time_service.h"

#include <stdio.h>

/* Guard padrão do ESP-IDF: só compila a parte de tarefa no hardware real.
 * No PC (sem ESP_PLATFORM), o firmware continua 100% portátil. */
#if defined(ESP_PLATFORM)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

/* HAL ativo — injetado pela porta (mock no PC, board na placa). */
static const runesos_hal_t *s_hal = NULL;

void runesos_fw_set_hal(const runesos_hal_t *hal)
{
    s_hal = hal;
}

const runesos_hal_t *runesos_fw_get_hal(void)
{
    return s_hal;
}

/* Health-check periódico (apenas no hardware real).
 * Os services são stateless: leem o HAL sob demanda, então a UI
 * já puxa as atualizações via lv_timer. Esta tarefa só garante que
 * o HAL está íntegro e loga se faltar callback essencial. */
#if defined(ESP_PLATFORM)
static void runesos_fw_poll_task(void *arg)
{
    (void)arg;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5000));

        if (s_hal == NULL) {
            printf("[RUNESOS_FW] HAL não registrado.\n");
            continue;
        }
        if (!s_hal->get_time || !s_hal->get_battery || !s_hal->get_radio) {
            printf("[RUNESOS_FW] ALERTA: HAL incompleto — "
                   "faltam callbacks essenciais.\n");
        }
    }
}
#endif

void runesos_fw_init(void)
{
    if (s_hal == NULL) {
        printf("[RUNESOS_FW] Nenhum HAL registrado. "
               "Chame runesos_fw_set_hal() antes de init.\n");
        return;
    }

    /* Services são stateless: delegam ao HAL sob demanda.
     * Aqui apenas garantimos que os callbacks essenciais existem. */
    if (!s_hal->get_time || !s_hal->get_battery || !s_hal->get_radio) {
        printf("[RUNESOS_FW] ALERTA: HAL incompleto no init.\n");
    }

#if defined(ESP_PLATFORM)
    xTaskCreate(runesos_fw_poll_task, "runesos_fw", 4096, NULL, 3, NULL);
#endif

    printf("[RUNESOS_FW] Firmware inicializado.\n");
}