#include "GlobalUI.h"
#include "AppManager.h"
#include <lvgl.h>
#include "BSP.h"

lv_obj_t *GlobalUI::sidebar = nullptr;
lv_obj_t *GlobalUI::toggleBtn = nullptr;
lv_obj_t *GlobalUI::homeBtn = nullptr;
bool GlobalUI::sidebarOpen = false;
lv_obj_t* GlobalUI::customButtons[MAX_SIDEBAR_BUTTONS] = {nullptr};
int GlobalUI::customButtonCount = 0;

#if LV_USE_PERF_MONITOR || LV_USE_MEM_MONITOR
#include <lvgl.h>

static lv_obj_t* find_perf_monitor_label() {
    lv_obj_t* layer = lv_layer_sys();
    lv_obj_t* child = lv_obj_get_child(layer, 0);
    while (child) {
        if (lv_obj_get_class(child) == &lv_label_class) {
            lv_color_t bg_color = lv_obj_get_style_bg_color(child, 0);
            if (LV_COLOR_GET_R(bg_color) == 0 && LV_COLOR_GET_G(bg_color) == 0 && LV_COLOR_GET_B(bg_color) == 0) {
                lv_opa_t bg_opa = lv_obj_get_style_bg_opa(child, 0);
                if (bg_opa == LV_OPA_50) {
                    return child;
                }
            }
        }
        child = lv_obj_get_child(layer, lv_obj_get_child_id(child) + 1);
    }
    return NULL;
}

static lv_obj_t* find_memory_monitor_label() {
    lv_obj_t* layer = lv_layer_sys();
    lv_obj_t* child = lv_obj_get_child(layer, 0);
    while (child) {
        if (lv_obj_get_class(child) == &lv_label_class) {
            lv_color_t bg_color = lv_obj_get_style_bg_color(child, 0);
            if (LV_COLOR_GET_R(bg_color) == 0 && LV_COLOR_GET_G(bg_color) == 0 && LV_COLOR_GET_B(bg_color) == 0) {
                lv_opa_t bg_opa = lv_obj_get_style_bg_opa(child, 0);
                if (bg_opa == LV_OPA_50) {
                    lv_align_t align = lv_obj_get_style_align(child, 0);
                    if (align == LV_ALIGN_BOTTOM_LEFT) {
                        return child;
                    }
                }
            }
        }
        child = lv_obj_get_child(layer, lv_obj_get_child_id(child) + 1);
    }
    return NULL;
}
#endif

void home_btn_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        AppMgr.switchToHome();
    }
}

typedef struct {
    sidebar_btn_callback_t callback;
    void* user_data;
} custom_btn_data_t;

void custom_btn_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        lv_obj_t* btn = lv_event_get_target(e);
        custom_btn_data_t* data = (custom_btn_data_t*)lv_obj_get_user_data(btn);
        if (data && data->callback) {
            data->callback(data->user_data);
        }
    }
}

void toggle_sidebar(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (GlobalUI::sidebarOpen) {
            lv_obj_set_pos(GlobalUI::sidebar, -50, 20);
            lv_label_set_text(lv_obj_get_child(GlobalUI::toggleBtn, 0), LV_SYMBOL_RIGHT);
            if (GlobalUI::homeBtn) {
                lv_obj_add_flag(GlobalUI::homeBtn, LV_OBJ_FLAG_HIDDEN);
            }
            for (int i = 0; i < GlobalUI::customButtonCount; i++) {
                if (GlobalUI::customButtons[i]) {
                    lv_obj_add_flag(GlobalUI::customButtons[i], LV_OBJ_FLAG_HIDDEN);
                }
            }
            #if LV_USE_PERF_MONITOR
            lv_obj_t* perf_label = find_perf_monitor_label();
            if (perf_label) {
                lv_obj_add_flag(perf_label, LV_OBJ_FLAG_HIDDEN);
            }
            #endif
            #if LV_USE_MEM_MONITOR
            lv_obj_t* mem_label = find_memory_monitor_label();
            if (mem_label) {
                lv_obj_add_flag(mem_label, LV_OBJ_FLAG_HIDDEN);
            }
            #endif
        } else {
            lv_obj_set_pos(GlobalUI::sidebar, 0, 20);
            lv_label_set_text(lv_obj_get_child(GlobalUI::toggleBtn, 0), LV_SYMBOL_LEFT);
            if (GlobalUI::homeBtn) {
                lv_obj_clear_flag(GlobalUI::homeBtn, LV_OBJ_FLAG_HIDDEN);
            }
            for (int i = 0; i < GlobalUI::customButtonCount; i++) {
                if (GlobalUI::customButtons[i]) {
                    lv_obj_clear_flag(GlobalUI::customButtons[i], LV_OBJ_FLAG_HIDDEN);
                }
            }
            #if LV_USE_PERF_MONITOR
            lv_obj_t* perf_label = find_perf_monitor_label();
            if (perf_label) {
                lv_obj_clear_flag(perf_label, LV_OBJ_FLAG_HIDDEN);
            }
            #endif
            #if LV_USE_MEM_MONITOR
            lv_obj_t* mem_label = find_memory_monitor_label();
            if (mem_label) {
                lv_obj_clear_flag(mem_label, LV_OBJ_FLAG_HIDDEN);
            }
            #endif
        }
        GlobalUI::sidebarOpen = !GlobalUI::sidebarOpen;
    }
}

void GlobalUI::init() {
    if (!toggleBtn) {
        toggleBtn = lv_btn_create(lv_layer_top());
        lv_obj_set_size(toggleBtn, 50, 20);
        lv_obj_align(toggleBtn, LV_ALIGN_TOP_LEFT, 0, 0);
        lv_obj_add_event_cb(toggleBtn, toggle_sidebar, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(toggleBtn, lv_color_make(0x80, 0x80, 0x80), 0);
        lv_obj_set_style_border_width(toggleBtn, 0, 0);
        lv_obj_set_style_radius(toggleBtn, 0, 0);

        lv_obj_t *arrow = lv_label_create(toggleBtn);
        lv_label_set_text(arrow, sidebarOpen ? LV_SYMBOL_LEFT : LV_SYMBOL_RIGHT);
        lv_obj_center(arrow);
        lv_obj_set_style_text_color(arrow, lv_color_white(), 0);
        lv_obj_set_style_text_font(arrow, &lv_font_montserrat_14, 0);
    }

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
        lv_obj_set_style_pad_all(sidebar, 0, 0);
        
        #if LV_USE_PERF_MONITOR
        lv_obj_t* perf_label = find_perf_monitor_label();
        if (perf_label) {
            if (sidebarOpen) {
                lv_obj_clear_flag(perf_label, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(perf_label, LV_OBJ_FLAG_HIDDEN);
            }
        }
        #endif
        
        #if LV_USE_MEM_MONITOR
        lv_obj_t* mem_label = find_memory_monitor_label();
        if (mem_label) {
            if (sidebarOpen) {
                lv_obj_clear_flag(mem_label, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(mem_label, LV_OBJ_FLAG_HIDDEN);
            }
        }
        #endif
    } else {
        lv_obj_set_pos(sidebar, sidebarOpen ? 0 : -50, 20);
    }

    if (!homeBtn) {
        homeBtn = lv_btn_create(lv_layer_top());
        lv_obj_set_size(homeBtn, 50, 20);
        lv_obj_set_pos(homeBtn, 0, 20);
        lv_obj_add_event_cb(homeBtn, home_btn_cb, LV_EVENT_CLICKED, NULL);
        lv_obj_set_style_bg_color(homeBtn, lv_color_make(0x80, 0x80, 0x80), 0);
        lv_obj_set_style_border_width(homeBtn, 0, 0);
        lv_obj_set_style_radius(homeBtn, 0, 0);
        
        lv_obj_t *homeLabel = lv_label_create(homeBtn);
        lv_label_set_text(homeLabel, LV_SYMBOL_HOME);
        lv_obj_center(homeLabel);
        lv_obj_set_style_text_color(homeLabel, lv_color_white(), 0);
        lv_obj_set_style_text_font(homeLabel, &lv_font_montserrat_14, 0);
        
        if (!sidebarOpen) {
            lv_obj_add_flag(homeBtn, LV_OBJ_FLAG_HIDDEN);
        }
    }

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
    clearSidebarButtons();
    
    if (toggleBtn) {
        lv_obj_del(toggleBtn);
        toggleBtn = nullptr;
    }
    if (homeBtn) {
        lv_obj_del(homeBtn);
        homeBtn = nullptr;
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

lv_obj_t* GlobalUI::addSidebarButton(const char* symbol, sidebar_btn_callback_t callback, void* user_data) {
    if (customButtonCount >= MAX_SIDEBAR_BUTTONS) {
        return nullptr;
    }
    
    lv_obj_t* btn = lv_btn_create(lv_layer_top());
    lv_obj_set_size(btn, 50, 30);
    lv_obj_set_pos(btn, 0, 40 + customButtonCount * 30);
    lv_obj_set_style_bg_color(btn, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 0, 0);
    
    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, symbol);
    lv_obj_center(label);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    
    custom_btn_data_t* data = (custom_btn_data_t*)malloc(sizeof(custom_btn_data_t));
    if (data) {
        data->callback = callback;
        data->user_data = user_data;
        lv_obj_set_user_data(btn, data);
    }
    
    lv_obj_add_event_cb(btn, custom_btn_cb, LV_EVENT_CLICKED, NULL);
    
    if (!sidebarOpen) {
        lv_obj_add_flag(btn, LV_OBJ_FLAG_HIDDEN);
    }
    
    customButtons[customButtonCount++] = btn;
    
    Serial.printf("[GlobalUI] Added sidebar button %d at y=%d\n", customButtonCount - 1, 40 + (customButtonCount - 1) * 30);
    
    return btn;
}

void GlobalUI::removeSidebarButton(lv_obj_t* btn) {
    if (!btn) return;
    
    custom_btn_data_t* data = (custom_btn_data_t*)lv_obj_get_user_data(btn);
    if (data) {
        free(data);
    }
    
    for (int i = 0; i < customButtonCount; i++) {
        if (customButtons[i] == btn) {
            lv_obj_del(btn);
            for (int j = i; j < customButtonCount - 1; j++) {
                customButtons[j] = customButtons[j + 1];
                if (customButtons[j]) {
                    lv_obj_set_pos(customButtons[j], 0, 40 + j * 30);
                }
            }
            customButtons[customButtonCount - 1] = nullptr;
            customButtonCount--;
            break;
        }
    }
}

void GlobalUI::clearSidebarButtons() {
    for (int i = 0; i < customButtonCount; i++) {
        if (customButtons[i]) {
            custom_btn_data_t* data = (custom_btn_data_t*)lv_obj_get_user_data(customButtons[i]);
            if (data) {
                free(data);
            }
            lv_obj_del(customButtons[i]);
            customButtons[i] = nullptr;
        }
    }
    customButtonCount = 0;
}

GlobalUI GlobalUI::instance;

GlobalUI &GlobalUI::getInstance() {
    return instance;
}
