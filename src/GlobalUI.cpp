#include "GlobalUI.h"
#include <lvgl.h>
#include "BSP.h"

lv_obj_t *GlobalUI::sidebar = nullptr;
lv_obj_t *GlobalUI::toggleBtn = nullptr;
bool GlobalUI::sidebarOpen = false;

void toggle_sidebar(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (GlobalUI::sidebarOpen) {
            lv_obj_set_pos(GlobalUI::sidebar, -50, 20);
            lv_label_set_text(lv_obj_get_child(GlobalUI::toggleBtn, 0), LV_SYMBOL_RIGHT);
        } else {
            lv_obj_set_pos(GlobalUI::sidebar, 0, 20);
            lv_label_set_text(lv_obj_get_child(GlobalUI::toggleBtn, 0), LV_SYMBOL_LEFT);
        }
        GlobalUI::sidebarOpen = !GlobalUI::sidebarOpen;
    }
}

void GlobalUI::init() {
    // Create toggle button
    if (!toggleBtn) {
        toggleBtn = lv_btn_create(lv_layer_top());
        lv_obj_set_size(toggleBtn, 50, 20);
        lv_obj_align(toggleBtn, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_add_event_cb(toggleBtn, toggle_sidebar, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(toggleBtn, lv_color_make(0x80, 0x80, 0x80), 0);
        lv_obj_set_style_border_width(toggleBtn, 0, 0);
        lv_obj_set_style_radius(toggleBtn, 0, 0);

        // Add arrow label
        lv_obj_t *arrow = lv_label_create(toggleBtn);
        lv_label_set_text(arrow, sidebarOpen ? LV_SYMBOL_LEFT : LV_SYMBOL_RIGHT);
        lv_obj_center(arrow);
        lv_obj_set_style_text_color(arrow, lv_color_white(), 0);
        lv_obj_set_style_text_font(arrow, &lv_font_montserrat_14, 0);
    }

    // Create sidebar
    if (!sidebar) {
        sidebar = lv_obj_create(lv_layer_top());
        lv_obj_set_size(sidebar, 50, lv_disp_get_ver_res(lv_disp_get_default()) - 20);
        lv_obj_align(sidebar, LV_ALIGN_TOP_LEFT, sidebarOpen ? 0 : -50, 20);
        lv_obj_set_style_bg_color(sidebar, lv_color_make(0xD0, 0xD0, 0xD0), 0);
        lv_obj_set_style_border_width(sidebar, 0, 0);
        lv_obj_set_style_radius(sidebar, 0, 0);
        lv_obj_set_style_shadow_width(sidebar, 2, 0);
        lv_obj_set_style_shadow_color(sidebar, lv_color_make(0x00, 0x00, 0x00), 0);
        lv_obj_set_style_shadow_ofs_x(sidebar, 2, 0);
        lv_obj_set_style_shadow_ofs_y(sidebar, 0, 0);
        lv_obj_set_style_shadow_opa(sidebar, 100, 0);
    } else {
        // Update sidebar position
        lv_obj_set_pos(sidebar, sidebarOpen ? 0 : -50, 20);
    }

    // Add transition for smooth sliding
    static const lv_style_prop_t props[] = {
        LV_STYLE_TRANSLATE_X,
        LV_STYLE_TRANSLATE_Y,
        LV_STYLE_OPA,
        LV_STYLE_PROP_INV
    };
    
    lv_style_transition_dsc_t transition;
    lv_style_transition_dsc_init(&transition, props, lv_anim_path_linear, 200, 0, NULL);
    lv_obj_set_style_transition(sidebar, &transition, 0);
}

void GlobalUI::deinit() {
    if (toggleBtn) {
        lv_obj_del(toggleBtn);
        toggleBtn = nullptr;
    }
    if (sidebar) {
        lv_obj_del(sidebar);
        sidebar = nullptr;
    }
}

bool GlobalUI::isSidebarOpen() {
    return sidebarOpen;
}

void GlobalUI::toggleSidebar() {
    if (toggleBtn) {
        lv_event_send(toggleBtn, LV_EVENT_CLICKED, NULL);
    }
}

GlobalUI GlobalUI::instance;

GlobalUI &GlobalUI::getInstance() {
    return instance;
}