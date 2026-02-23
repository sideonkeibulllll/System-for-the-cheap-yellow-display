#ifndef GLOBALUI_H
#define GLOBALUI_H

#include <lvgl.h>

class GlobalUI {
private:
    static GlobalUI instance;
    GlobalUI() = default;
    ~GlobalUI() = default;

    static lv_obj_t *sidebar;
    static lv_obj_t *toggleBtn;
    static lv_obj_t *homeBtn;
    static bool sidebarOpen;

    friend void toggle_sidebar(lv_event_t *e);
    friend void home_btn_cb(lv_event_t *e);

public:
    GlobalUI(const GlobalUI &) = delete;
    GlobalUI &operator=(const GlobalUI &) = delete;

    static GlobalUI &getInstance();

    void init();
    void deinit();
    bool isSidebarOpen();
    void toggleSidebar();
};

#endif
