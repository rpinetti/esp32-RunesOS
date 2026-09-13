#ifndef LVGL_COMPAT_H
#define LVGL_COMPAT_H

#include "lvgl.h"

/* LVGL v8 <-> v9 Compatibility Layer */
#if defined(LVGL_VERSION_MAJOR) && (LVGL_VERSION_MAJOR < 9) || (defined(LV_VERSION_CHECK) && !LV_VERSION_CHECK(9,0,0))

#ifndef lv_obj_delete
#define lv_obj_delete(obj) lv_obj_del(obj)
#endif

#ifndef lv_timer_delete
#define lv_timer_delete(timer) lv_timer_del(timer)
#endif

#ifndef lv_anim_set_completed_cb
#define lv_anim_set_completed_cb(anim, cb) lv_anim_set_ready_cb(anim, cb)
#endif

#ifndef lv_indev_active
#define lv_indev_active() lv_indev_get_act()
#endif

#ifndef lv_display_get_vertical_resolution
#define lv_display_get_vertical_resolution(disp) LV_VER_RES
#endif

#ifndef lv_display_get_horizontal_resolution
#define lv_display_get_horizontal_resolution(disp) LV_HOR_RES
#endif

#ifndef lv_obj_set_scrollable
#define lv_obj_set_scrollable(obj, enable) do { \
    if (enable) lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE); \
    else lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE); \
} while(0)
#endif

#endif /* LVGL v8 compatibility */

#endif // LVGL_COMPAT_H
