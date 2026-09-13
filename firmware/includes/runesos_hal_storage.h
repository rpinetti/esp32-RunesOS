#ifndef RUNESOS_HAL_STORAGE_H
#define RUNESOS_HAL_STORAGE_H

#include <stdbool.h>
#include <stddef.h>

#define NOTES_DIR        "notes"    /* caminho lógico: cada porta mapeia para o seu root */
#define NOTES_MAX_NAME   64         /* tamanho de um nome de arquivo */
#define NOTES_MAX_NOTES  32         /* máximo de notas listadas */
#define NOTES_MAX_LEN    8192       /* limite de tamanho de uma nota (bytes) */

typedef struct {
    bool (*file_exists)(const char *path);
    bool (*file_read)(const char *path, char *buf, size_t max_len, size_t *out_len);
    bool (*file_write)(const char *path, const char *buf, size_t len);
    bool (*file_delete)(const char *path);
    int  (*dir_list)(const char *dir, char names[][NOTES_MAX_NAME], int max_entries);
} runesos_hal_storage_t;

/* A porta ativa fornece o ponteiro (PC → stdio, placa → LittleFS/SD) */
const runesos_hal_storage_t *runesos_hal_storage_get(void);

#endif