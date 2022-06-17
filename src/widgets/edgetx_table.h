#ifndef EDGETX_TABLE_H
#define EDGETX_TABLE_H
#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_obj_class_t lv_edgetx_table_class;
lv_obj_t* edgetx_table_create(lv_obj_t* parent);

#ifdef __cplusplus
};
#endif

#endif
