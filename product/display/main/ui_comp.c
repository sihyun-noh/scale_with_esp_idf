// SquareLine LVGL GENERATED FILE
// EDITOR VERSION: SquareLine Studio 1.1.1
// LVGL VERSION: 8.3.3
// PROJECT: SquareLine_Project


#include "ui.h"
#include "ui_helpers.h"
#include "ui_comp.h"

uint32_t LV_EVENT_GET_COMP_CHILD;

typedef struct {
uint32_t child_idx;
lv_obj_t* child;
} ui_comp_get_child_t;

lv_obj_t * ui_comp_get_child(lv_obj_t *comp, uint32_t child_idx) {
ui_comp_get_child_t info;
info.child = NULL;
info.child_idx = child_idx;
lv_event_send(comp, LV_EVENT_GET_COMP_CHILD, &info);
 return info.child;
}

void get_component_child_event_cb(lv_event_t* e) {
lv_obj_t** c = lv_event_get_user_data(e);
ui_comp_get_child_t* info = lv_event_get_param(e);
info->child = c[info->child_idx];
 }

void del_component_child_event_cb(lv_event_t* e) {
lv_obj_t** c = lv_event_get_user_data(e);
lv_mem_free(c); 
}


// COMPONENT Screen2 Keyboard1

lv_obj_t *ui_Screen2_Keyboard1_create(lv_obj_t *comp_parent) {

lv_obj_t *cui_Screen2_Keyboard1;
cui_Screen2_Keyboard1 = lv_keyboard_create(comp_parent);
lv_keyboard_set_mode(cui_Screen2_Keyboard1,LV_KEYBOARD_MODE_NUMBER);
lv_obj_set_width( cui_Screen2_Keyboard1, 300);
lv_obj_set_height( cui_Screen2_Keyboard1, 141);
lv_obj_set_x( cui_Screen2_Keyboard1, -77 );
lv_obj_set_y( cui_Screen2_Keyboard1, 77 );
lv_obj_set_align( cui_Screen2_Keyboard1, LV_ALIGN_CENTER );

lv_obj_t ** children = lv_mem_alloc(sizeof(lv_obj_t *) * _UI_COMP_SCREEN2_KEYBOARD1_NUM);
children[UI_COMP_SCREEN2_KEYBOARD1_SCREEN2_KEYBOARD1] = cui_Screen2_Keyboard1;
lv_obj_add_event_cb(cui_Screen2_Keyboard1, get_component_child_event_cb, LV_EVENT_GET_COMP_CHILD, children);
lv_obj_add_event_cb(cui_Screen2_Keyboard1, del_component_child_event_cb, LV_EVENT_DELETE, children);
return cui_Screen2_Keyboard1; 
}

