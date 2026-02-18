#ifndef DEMOAPP_H
#define DEMOAPP_H

#include "AppManager.h"
#include "BSP.h"
#include "PowerManager.h"

class DemoApp : public BaseApp {
private:
    lv_obj_t* labelCounter;
    lv_obj_t* btnInc;
    lv_obj_t* btnDec;
    lv_obj_t* btnBack;
    lv_obj_t* labelPerf;
    
    int _counter;
    uint32_t _lastUpdate;
    
    static void btn_inc_cb(lv_event_t* e);
    static void btn_dec_cb(lv_event_t* e);
    static void back_btn_cb(lv_event_t* e);
    
protected:
    virtual bool createUI() override;
    virtual void saveState() override;
    virtual bool loadState() override;
    
public:
    DemoApp();
    virtual ~DemoApp() {}
    
    virtual void onUpdate() override;
    virtual app_info_t getInfo() const override;
};

BaseApp* createDemoApp();

#endif
