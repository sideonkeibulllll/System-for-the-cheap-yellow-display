#include "DemoApp.h"
#include "AppManager.h"
#include "Performance.h"

DemoApp::DemoApp() : BaseApp("Demo") {
    labelCounter = nullptr;
    btnInc = nullptr;
    btnDec = nullptr;
    btnBack = nullptr;
    labelPerf = nullptr;
    _counter = 0;
    _lastUpdate = 0;
}

bool DemoApp::createUI() {
    lv_obj_t* scr = getScreen();
    
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_PLAY " Demo App");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, "Counter Demo - Test App Lifecycle");
    lv_obj_set_style_text_color(info, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_12, 0);
    lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 30);
    
    labelCounter = lv_label_create(scr);
    lv_label_set_text_fmt(labelCounter, "%d", _counter);
    lv_obj_set_style_text_color(labelCounter, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelCounter, &lv_font_montserrat_48, 0);
    lv_obj_align(labelCounter, LV_ALIGN_CENTER, 0, -20);
    
    lv_obj_t* btnContainer = lv_obj_create(scr);
    lv_obj_set_size(btnContainer, 200, 50);
    lv_obj_set_style_bg_opa(btnContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btnContainer, 0, 0);
    lv_obj_set_style_pad_all(btnContainer, 0, 0);
    lv_obj_align(btnContainer, LV_ALIGN_CENTER, 0, 40);
    
    btnDec = lv_btn_create(btnContainer);
    lv_obj_set_size(btnDec, 80, 40);
    lv_obj_align(btnDec, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_add_event_cb(btnDec, btn_dec_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnDec, lv_color_make(0x80, 0x00, 0x00), 0);
    
    lv_obj_t* decLabel = lv_label_create(btnDec);
    lv_label_set_text(decLabel, LV_SYMBOL_MINUS " Minus");
    lv_obj_set_style_text_font(decLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(decLabel);
    
    btnInc = lv_btn_create(btnContainer);
    lv_obj_set_size(btnInc, 80, 40);
    lv_obj_align(btnInc, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(btnInc, btn_inc_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnInc, lv_color_make(0x00, 0x80, 0x00), 0);
    
    lv_obj_t* incLabel = lv_label_create(btnInc);
    lv_label_set_text(incLabel, LV_SYMBOL_PLUS " Plus");
    lv_obj_set_style_text_font(incLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(incLabel);
    
    labelPerf = lv_label_create(scr);
    lv_label_set_text(labelPerf, "FPS: -- | Heap: -- KB");
    lv_obj_set_style_text_color(labelPerf, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelPerf, &lv_font_montserrat_12, 0);
    lv_obj_align(labelPerf, LV_ALIGN_BOTTOM_MID, 0, -50);
    
    btnBack = lv_btn_create(scr);
    lv_obj_set_size(btnBack, 80, 35);
    lv_obj_align(btnBack, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_event_cb(btnBack, back_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnBack, lv_color_make(0x40, 0x40, 0x80), 0);
    
    lv_obj_t* btnLabel = lv_label_create(btnBack);
    lv_label_set_text(btnLabel, LV_SYMBOL_LEFT " Back");
    lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_14, 0);
    lv_obj_center(btnLabel);
    
    return true;
}

void DemoApp::btn_inc_cb(lv_event_t* e) {
    DemoApp* app = (DemoApp*)lv_event_get_user_data(e);
    app->_counter++;
    lv_label_set_text_fmt(app->labelCounter, "%d", app->_counter);
    Power.resetIdleTimer();
    bsp_rgb_led_set(0, 255, 0);
}

void DemoApp::btn_dec_cb(lv_event_t* e) {
    DemoApp* app = (DemoApp*)lv_event_get_user_data(e);
    app->_counter--;
    lv_label_set_text_fmt(app->labelCounter, "%d", app->_counter);
    Power.resetIdleTimer();
    bsp_rgb_led_set(255, 0, 0);
}

void DemoApp::back_btn_cb(lv_event_t* e) {
    AppMgr.switchToHome();
}

void DemoApp::onUpdate() {
    uint32_t now = millis();
    if (now - _lastUpdate >= 1000) {
        _lastUpdate = now;
        
        perf_stats_t stats = Perf.getStats();
        lv_label_set_text_fmt(labelPerf, 
            "FPS: %u | Heap: %u KB",
            stats.fps,
            stats.freeHeap / 1024
        );
    }
}

void DemoApp::saveState() {
    Serial.printf("[Demo] Saving counter value: %d\n", _counter);
}

bool DemoApp::loadState() {
    Serial.printf("[Demo] Loaded counter value: %d\n", _counter);
    if (labelCounter) {
        lv_label_set_text_fmt(labelCounter, "%d", _counter);
    }
    return true;
}

app_info_t DemoApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "Demo", APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, LV_SYMBOL_PLAY);
    info.type = APP_TYPE_USER;
    info.enabled = true;
    return info;
}

BaseApp* createDemoApp() {
    return new DemoApp();
}
