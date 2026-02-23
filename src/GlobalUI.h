#ifndef GLOBALUI_H
#define GLOBALUI_H

#include <lvgl.h>

#define MAX_SIDEBAR_BUTTONS 8

typedef void (*sidebar_btn_callback_t)(void* user_data);

class GlobalUI {
private:
    static GlobalUI instance;
    GlobalUI() = default;
    ~GlobalUI() = default;

    static lv_obj_t *sidebar;
    static lv_obj_t *toggleBtn;
    static lv_obj_t *homeBtn;
    static bool sidebarOpen;
    
    static lv_obj_t* customButtons[MAX_SIDEBAR_BUTTONS];
    static int customButtonCount;

    friend void toggle_sidebar(lv_event_t *e);
    friend void home_btn_cb(lv_event_t *e);
    friend void custom_btn_cb(lv_event_t *e);

public:
    GlobalUI(const GlobalUI &) = delete;
    GlobalUI &operator=(const GlobalUI &) = delete;

    static GlobalUI &getInstance();

    void init();
    void deinit();
    bool isSidebarOpen();
    void toggleSidebar();
    
    lv_obj_t* addSidebarButton(const char* symbol, sidebar_btn_callback_t callback, void* user_data);
    void removeSidebarButton(lv_obj_t* btn);
    void clearSidebarButtons();
};

#endif
