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
            lv_obj_set_pos(GlobalUI::sidebar, -50, 0);
            lv_label_set_text(lv_obj_get_child(GlobalUI::toggleBtn, 0), LV_SYMBOL_RIGHT);
        } else {
            lv_obj_set_pos(GlobalUI::sidebar, 0, 0);
            lv_label_set_text(lv_obj_get_child(GlobalUI::toggleBtn, 0), LV_SYMBOL_LEFT);
        }
        GlobalUI::sidebarOpen = !GlobalUI::sidebarOpen;
        // Ensure button is always on top
        lv_obj_move_foreground(GlobalUI::toggleBtn);
    }
}

void GlobalUI::init() {
    // Create toggle button
    if (!toggleBtn) {
        toggleBtn = lv_btn_create(lv_scr_act());
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
    } else {
        // Bring button to front
        lv_obj_move_foreground(toggleBtn);
    }

    // Create sidebar
    if (!sidebar) {
        sidebar = lv_obj_create(lv_scr_act());
        lv_obj_set_size(sidebar, 50, lv_disp_get_ver_res(lv_disp_get_default()));
        lv_obj_align(sidebar, LV_ALIGN_TOP_LEFT, sidebarOpen ? 0 : -50, 0);
        lv_obj_set_style_bg_color(sidebar, lv_color_make(0xD0, 0xD0, 0xD0), 0);
        lv_obj_set_style_border_width(sidebar, 0, 0);
        lv_obj_set_style_radius(sidebar, 0, 0);
        lv_obj_set_style_shadow_width(sidebar, 2, 0);
        lv_obj_set_style_shadow_color(sidebar, lv_color_make(0x00, 0x00, 0x00), 0);
        lv_obj_set_style_shadow_ofs_x(sidebar, 2, 0);
        lv_obj_set_style_shadow_ofs_y(sidebar, 0, 0);
        lv_obj_set_style_shadow_opa(sidebar, 100, 0);

        // Add performance monitoring toggle switch
        lv_obj_t *toggleSwitch = lv_switch_create(sidebar);
        lv_obj_align(toggleSwitch, LV_ALIGN_TOP_MID, 0, 30);
        lv_obj_set_size(toggleSwitch, 40, 20);
        lv_obj_add_state(toggleSwitch, LV_STATE_CHECKED);
        lv_obj_add_event_cb(toggleSwitch, [](lv_event_t *e) {
            lv_event_code_t code = lv_event_get_code(e);
            if (code == LV_EVENT_VALUE_CHANGED) {
                lv_obj_t *switchObj = lv_event_get_target(e);
                bool enabled = lv_obj_has_state(switchObj, LV_STATE_CHECKED);
                // Toggle performance monitoring
                lv_obj_t *child = NULL;
                uint32_t i = 0;
                while ((child = lv_obj_get_child(lv_scr_act(), i++)) != NULL) {
                    if (lv_obj_get_class(child) == &lv_label_class) {
                        const char *text = lv_label_get_text(child);
                        if (text && (strstr(text, "FPS") || strstr(text, "Heap") || strstr(text, "LVGL") || strstr(text, "KB") || strstr(text, "frag") || strstr(text, "fps") || strstr(text, "cpu") || strstr(text, "used") || strstr(text, "free") || strstr(text, "mem"))) {
                            lv_obj_set_style_text_opa(child, enabled ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                        }
                    }
                }
                // Check all screens
                lv_disp_t *disp = lv_disp_get_default();
                lv_obj_t *scr = lv_disp_get_scr_act(disp);
                if (scr) {
                    lv_obj_t *child = NULL;
                    uint32_t i = 0;
                    while ((child = lv_obj_get_child(scr, i++)) != NULL) {
                        if (lv_obj_get_class(child) == &lv_label_class) {
                            const char *text = lv_label_get_text(child);
                            if (text && (strstr(text, "FPS") || strstr(text, "Heap") || strstr(text, "LVGL") || strstr(text, "KB") || strstr(text, "frag") || strstr(text, "fps") || strstr(text, "cpu") || strstr(text, "used") || strstr(text, "free") || strstr(text, "mem"))) {
                                lv_obj_set_style_text_opa(child, enabled ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
                            }
                        }
                    }
                }
            }
        }, LV_EVENT_ALL, NULL);

        // Add label for toggle switch
        lv_obj_t *toggleLabel = lv_label_create(sidebar);
        lv_obj_align(toggleLabel, LV_ALIGN_TOP_MID, 0, 60);
        lv_obj_set_style_text_font(toggleLabel, &lv_font_montserrat_8, 0);
        lv_obj_set_style_text_color(toggleLabel, lv_color_black(), 0);
        lv_obj_set_style_text_align(toggleLabel, LV_TEXT_ALIGN_CENTER, 0);
        lv_label_set_text(toggleLabel, "Perf Monitor");
    } else {
        // Update sidebar position
        lv_obj_set_pos(sidebar, sidebarOpen ? 0 : -50, 0);
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