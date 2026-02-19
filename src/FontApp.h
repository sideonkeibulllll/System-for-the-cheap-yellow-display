#ifndef FONTAPP_H
#define FONTAPP_H

#include "AppManager.h"
#include "BSP.h"

class FontApp : public BaseApp {
private:
    lv_obj_t* fontList;
    lv_obj_t* btnBack;
    static void back_btn_cb(lv_event_t* e);
    
protected:
    virtual bool createUI() override;
    
public:
    FontApp();
    virtual ~FontApp() {}
    
    virtual app_info_t getInfo() const override;
};

BaseApp* createFontApp();

#endif
