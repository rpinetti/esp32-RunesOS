/* notes_app.h */
#ifndef APP_NOTES_H
#define APP_NOTES_H

#include "runesos_app.h"   /* contrato: name, icon, create, destroy */
#include "lvgl.h"

lv_obj_t *notes_app_create(lv_obj_t *parent);
void      notes_app_destroy(lv_obj_t *obj);

extern const runesos_app_t notes_app;

#endif