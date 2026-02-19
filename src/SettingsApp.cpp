#include "SettingsApp.h"
#include "AppManager.h"

SettingsApp::SettingsApp() : BaseApp("Settings") {
    sliderBrightness = nullptr;
    labelBrightness = nullptr;
    ddBacklightMode = nullptr;
    labelCPUMode = nullptr;
    labelPowerStatus = nullptr;
    btnBack = nullptr;
    btnWiFiConfig = nullptr;
    _savedBrightness = 200;
    _savedBacklightMode = BACKLIGHT_MODE_MANUAL;
    _sliderDragging = false;
    _lastUpdateMs = 0;
    _lastDisplayedBrightness = 0;
    _lastDisplayedMode = 255;
    _lastDisplayedLdr = 0;
    _lastDisplayedIdle = 0;
}

bool SettingsApp::createUI() {
    lv_obj_t* scr = getScreen();
    
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_SETTINGS " Settings");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    lv_obj_t* labelBL = lv_label_create(scr);
    lv_label_set_text(labelBL, "Brightness:");
    lv_obj_set_style_text_color(labelBL, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelBL, &lv_font_montserrat_12, 0);
    lv_obj_align(labelBL, LV_ALIGN_TOP_LEFT, 10, 35);
    
    sliderBrightness = lv_slider_create(scr);
    lv_slider_set_range(sliderBrightness, 10, 255);
    lv_slider_set_value(sliderBrightness, _savedBrightness, LV_ANIM_OFF);
    lv_obj_set_width(sliderBrightness, 150);
    lv_obj_align(sliderBrightness, LV_ALIGN_TOP_LEFT, 100, 35);
    lv_obj_add_event_cb(sliderBrightness, brightness_slider_cb, LV_EVENT_ALL, this);
    
    labelBrightness = lv_label_create(scr);
    lv_label_set_text_fmt(labelBrightness, "%d", _savedBrightness);
    lv_obj_set_style_text_color(labelBrightness, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelBrightness, &lv_font_montserrat_12, 0);
    lv_obj_align(labelBrightness, LV_ALIGN_TOP_LEFT, 260, 35);
    
    lv_obj_t* labelMode = lv_label_create(scr);
    lv_label_set_text(labelMode, "BL Mode:");
    lv_obj_set_style_text_color(labelMode, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelMode, &lv_font_montserrat_12, 0);
    lv_obj_align(labelMode, LV_ALIGN_TOP_LEFT, 10, 65);
    
    ddBacklightMode = lv_dropdown_create(scr);
    lv_dropdown_set_options(ddBacklightMode, "Manual\nAuto\nOff");
    lv_dropdown_set_selected(ddBacklightMode, _savedBacklightMode);
    lv_obj_set_width(ddBacklightMode, 120);
    lv_obj_align(ddBacklightMode, LV_ALIGN_TOP_LEFT, 100, 60);
    lv_obj_add_event_cb(ddBacklightMode, backlight_mode_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* labelCPU = lv_label_create(scr);
    lv_label_set_text(labelCPU, "CPU Mode:");
    lv_obj_set_style_text_color(labelCPU, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelCPU, &lv_font_montserrat_12, 0);
    lv_obj_align(labelCPU, LV_ALIGN_TOP_LEFT, 10, 100);
    
    labelCPUMode = lv_label_create(scr);
    lv_label_set_text(labelCPUMode, "240 MHz");
    lv_obj_set_style_text_color(labelCPUMode, lv_color_make(0x00, 0xFF, 0xFF), 0);
    lv_obj_set_style_text_font(labelCPUMode, &lv_font_montserrat_12, 0);
    lv_obj_align(labelCPUMode, LV_ALIGN_TOP_LEFT, 100, 100);
    
    lv_obj_t* labelPwrTitle = lv_label_create(scr);
    lv_label_set_text(labelPwrTitle, "Power Status:");
    lv_obj_set_style_text_color(labelPwrTitle, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelPwrTitle, &lv_font_montserrat_12, 0);
    lv_obj_align(labelPwrTitle, LV_ALIGN_TOP_LEFT, 10, 130);
    
    labelPowerStatus = lv_label_create(scr);
    lv_label_set_text(labelPowerStatus, "State: Active\nIdle: 0s\nLDR: 0");
    lv_obj_set_style_text_color(labelPowerStatus, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelPowerStatus, &lv_font_montserrat_12, 0);
    lv_obj_align(labelPowerStatus, LV_ALIGN_TOP_LEFT, 10, 155);
    
    btnWiFiConfig = lv_btn_create(scr);
    lv_obj_set_size(btnWiFiConfig, 140, 35);
    lv_obj_align(btnWiFiConfig, LV_ALIGN_TOP_RIGHT, -10, 130);
    lv_obj_add_event_cb(btnWiFiConfig, wifi_config_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnWiFiConfig, lv_color_make(0x00, 0x60, 0x80), 0);
    
    lv_obj_t* btnWiFiLabel = lv_label_create(btnWiFiConfig);
    lv_label_set_text(btnWiFiLabel, LV_SYMBOL_WIFI " WiFi Config");
    lv_obj_set_style_text_font(btnWiFiLabel, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(btnWiFiLabel, lv_color_white(), 0);
    lv_obj_center(btnWiFiLabel);
    
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

void SettingsApp::brightness_slider_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    lv_obj_t* slider = lv_event_get_target(e);
    
    if (code == LV_EVENT_PRESSING || code == LV_EVENT_PRESSED) {
        app->_sliderDragging = true;
    } else if (code == LV_EVENT_RELEASED) {
        app->_sliderDragging = false;
    }
    
    int value = lv_slider_get_value(slider);
    app->_savedBrightness = value;
    
    if (Power.getStatus().backlightMode == BACKLIGHT_MODE_MANUAL) {
        Power.setBacklight(value);
    }
    
    lv_label_set_text_fmt(app->labelBrightness, "%d", value);
}

void SettingsApp::backlight_mode_cb(lv_event_t* e) {
    SettingsApp* app = (SettingsApp*)lv_event_get_user_data(e);
    lv_obj_t* dd = lv_event_get_target(e);
    int selected = lv_dropdown_get_selected(dd);
    
    app->_savedBacklightMode = (backlight_mode_t)selected;
    Power.setBacklightMode((backlight_mode_t)selected);
    
    if (selected == BACKLIGHT_MODE_MANUAL) {
        Power.setBacklight(app->_savedBrightness);
    }
}

void SettingsApp::back_btn_cb(lv_event_t* e) {
    AppMgr.switchToHome();
}

void SettingsApp::wifi_config_btn_cb(lv_event_t* e) {
    AppMgr.switchToApp("WiFiConfig");
}

void SettingsApp::onUpdate() {
    uint32_t now = millis();
    
    if (now - _lastUpdateMs < 500) {
        return;
    }
    _lastUpdateMs = now;
    
    power_status_t& status = Power.getStatus();
    
    uint16_t currentLdr = status.ldrValue;
    uint32_t currentIdle = status.idleTimeMs / 1000;
    uint8_t currentBrightness = status.backlightLevel;
    uint8_t currentMode = status.backlightMode;
    
    if (currentLdr != _lastDisplayedLdr || currentIdle != _lastDisplayedIdle) {
        const char* stateStr = status.state == POWER_STATE_ACTIVE ? "Active" :
                               status.state == POWER_STATE_IDLE ? "Idle" : "Sleep";
        
        lv_label_set_text_fmt(labelPowerStatus, 
            "State: %s\nIdle: %us\nLDR: %d",
            stateStr,
            currentIdle,
            currentLdr
        );
        _lastDisplayedLdr = currentLdr;
        _lastDisplayedIdle = currentIdle;
    }
    
    if (!_sliderDragging && currentMode == BACKLIGHT_MODE_MANUAL && 
        currentBrightness != _lastDisplayedBrightness) {
        lv_slider_set_value(sliderBrightness, currentBrightness, LV_ANIM_OFF);
        lv_label_set_text_fmt(labelBrightness, "%d", currentBrightness);
        _savedBrightness = currentBrightness;
        _lastDisplayedBrightness = currentBrightness;
    }
    
    if (currentMode != _lastDisplayedMode) {
        lv_dropdown_set_selected(ddBacklightMode, currentMode);
        _lastDisplayedMode = currentMode;
    }
}

void SettingsApp::saveState() {
    _savedBrightness = Power.getStatus().backlightLevel;
    _savedBacklightMode = Power.getStatus().backlightMode;
    Serial.printf("[Settings] Saved state: brightness=%d, mode=%d\n", 
                  _savedBrightness, _savedBacklightMode);
}

bool SettingsApp::loadState() {
    _savedBrightness = Power.getStatus().backlightLevel;
    _savedBacklightMode = Power.getStatus().backlightMode;
    
    if (sliderBrightness) {
        lv_slider_set_value(sliderBrightness, _savedBrightness, LV_ANIM_OFF);
    }
    if (labelBrightness) {
        lv_label_set_text_fmt(labelBrightness, "%d", _savedBrightness);
    }
    if (ddBacklightMode) {
        lv_dropdown_set_selected(ddBacklightMode, _savedBacklightMode);
    }
    
    Serial.printf("[Settings] Loaded state: brightness=%d, mode=%d\n", 
                  _savedBrightness, _savedBacklightMode);
    return true;
}

app_info_t SettingsApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "Settings", APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, LV_SYMBOL_SETTINGS);
    info.type = APP_TYPE_SYSTEM;
    info.enabled = true;
    return info;
}

BaseApp* createSettingsApp() {
    return new SettingsApp();
}
