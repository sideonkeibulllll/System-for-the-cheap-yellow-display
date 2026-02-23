#ifndef CHATAPP_H
#define CHATAPP_H

#include "AppManager.h"

class ChatApp : public BaseApp {
private:
    lv_obj_t* _blankScreen;
    lv_obj_t* _floatBtn;
    
    bool createUI() override;
    void destroyUI() override;
    
    static void float_btn_cb(lv_event_t* e);
    
    void onFloatBtnClick();
    
public:
    ChatApp();
    ~ChatApp();
    
    void onUpdate() override;
    app_info_t getInfo() const override;
};

BaseApp* createChatApp();

#endif
