#ifndef SETTINGSAPP_H
#define SETTINGSAPP_H

#include "AppManager.h"
#include "BSP.h"
#include "PowerManager.h"

class SettingsApp : public BaseApp {
private:
    lv_obj_t* sliderBrightness;
    lv_obj_t* labelBrightness;
    lv_obj_t* ddBacklightMode;
    lv_obj_t* labelCPUMode;
    lv_obj_t* labelPowerStatus;
    lv_obj_t* btnBack;
    lv_obj_t* btnWiFiConfig;
    
    uint8_t _savedBrightness;
    backlight_mode_t _savedBacklightMode;
    bool _sliderDragging;
    
    uint32_t _lastUpdateMs;
    uint8_t _lastDisplayedBrightness;
    uint8_t _lastDisplayedMode;
    uint16_t _lastDisplayedLdr;
    uint32_t _lastDisplayedIdle;
    
    static void brightness_slider_cb(lv_event_t* e);
    static void backlight_mode_cb(lv_event_t* e);
    static void back_btn_cb(lv_event_t* e);
    static void wifi_config_btn_cb(lv_event_t* e);
    
protected:
    virtual bool createUI() override;
    virtual void saveState() override;
    virtual bool loadState() override;
    
public:
    SettingsApp();
    virtual ~SettingsApp() {}
    
    virtual void onUpdate() override;
    virtual app_info_t getInfo() const override;
};

BaseApp* createSettingsApp();

#endif
