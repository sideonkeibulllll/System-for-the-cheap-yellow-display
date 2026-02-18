#ifndef DEMOAPP_H
#define DEMOAPP_H

#include "AppManager.h"
#include "BSP.h"

class DemoApp : public BaseApp {
private:
    lv_obj_t* tabview;
    lv_obj_t* keyboard;
    lv_obj_t* activeTextarea;
    
    uint32_t _lastUpdateMs;
    int _animationPhase;
    bool _keyboardVisible;
    
    lv_obj_t* barProgress;
    lv_obj_t* arcGauge;
    lv_obj_t* chartWidget;
    lv_obj_t* labelClock;
    
    lv_chart_series_t* chartSeries1;
    lv_chart_series_t* chartSeries2;
    
    static void btn_click_cb(lv_event_t* e);
    static void slider_cb(lv_event_t* e);
    static void switch_cb(lv_event_t* e);
    static void dropdown_cb(lv_event_t* e);
    static void textarea_cb(lv_event_t* e);
    static void kb_event_cb(lv_event_t* e);
    static void colorwheel_cb(lv_event_t* e);
    static void list_btn_cb(lv_event_t* e);
    static void back_btn_cb(lv_event_t* e);
    
    void createBasicTab(lv_obj_t* parent);
    void createInputTab(lv_obj_t* parent);
    void createGraphicsTab(lv_obj_t* parent);
    void createListTab(lv_obj_t* parent);
    void createColorTab(lv_obj_t* parent);
    
    void showKeyboard(lv_obj_t* textarea);
    void hideKeyboard();
    
protected:
    virtual bool createUI() override;
    
public:
    DemoApp();
    virtual ~DemoApp() {}
    
    virtual void onUpdate() override;
    virtual app_info_t getInfo() const override;
};

BaseApp* createDemoApp();

#endif
