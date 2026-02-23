#ifndef CHATAPP_H
#define CHATAPP_H

#include "AppManager.h"

#define CHAT_INPUT_MAX_LEN      512

class ChatApp : public BaseApp {
private:
    lv_obj_t* _blankScreen;
    lv_obj_t* _floatBtn;
    lv_obj_t* _inputArea;
    lv_obj_t* _keyboard;
    
    bool _inputPanelVisible;
    
    bool createUI() override;
    void destroyUI() override;
    
    static void float_btn_cb(lv_event_t* e);
    static void input_focus_cb(lv_event_t* e);
    static void input_defocus_cb(lv_event_t* e);
    
    void onFloatBtnClick();
    void showInputPanel();
    void hideInputPanel();
    void toggleInputPanel();
    
public:
    ChatApp();
    ~ChatApp();
    
    void onUpdate() override;
    app_info_t getInfo() const override;
};

BaseApp* createChatApp();

#endif
