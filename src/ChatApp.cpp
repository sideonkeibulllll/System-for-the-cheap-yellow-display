#include "ChatApp.h"

ChatApp::ChatApp() : BaseApp("Chat") {
    _blankScreen = nullptr;
    _floatBtn = nullptr;
    _inputArea = nullptr;
    _keyboard = nullptr;
    _inputPanelVisible = false;
}

ChatApp::~ChatApp() {
}

bool ChatApp::createUI() {
    Serial.println("[ChatApp] createUI start");
    
    _blankScreen = lv_obj_create(_screen);
    lv_obj_set_size(_blankScreen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(_blankScreen, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(_blankScreen, 0, 0);
    lv_obj_set_style_pad_all(_blankScreen, 0, 0);
    
    _floatBtn = lv_btn_create(_screen);
    lv_obj_set_size(_floatBtn, 40, 40);
    lv_obj_align(_floatBtn, LV_ALIGN_TOP_RIGHT, -10, 10);
    lv_obj_set_style_bg_color(_floatBtn, lv_color_make(0x00, 0x80, 0xC0), 0);
    lv_obj_set_style_radius(_floatBtn, 20, 0);
    lv_obj_set_style_shadow_width(_floatBtn, 4, 0);
    lv_obj_set_style_shadow_color(_floatBtn, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_add_event_cb(_floatBtn, float_btn_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* btnIcon = lv_label_create(_floatBtn);
    lv_label_set_text(btnIcon, LV_SYMBOL_PLUS);
    lv_obj_set_style_text_color(btnIcon, lv_color_white(), 0);
    lv_obj_center(btnIcon);
    
    _inputArea = lv_textarea_create(_screen);
    lv_obj_set_size(_inputArea, 230, 60);
    lv_obj_align(_inputArea, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_textarea_set_max_length(_inputArea, CHAT_INPUT_MAX_LEN);
    lv_textarea_set_placeholder_text(_inputArea, "Input...");
    lv_obj_set_style_bg_color(_inputArea, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_bg_opa(_inputArea, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(_inputArea, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_border_color(_inputArea, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_border_width(_inputArea, 2, 0);
    lv_obj_set_style_radius(_inputArea, 6, 0);
    lv_obj_set_style_text_font(_inputArea, &lv_font_montserrat_14, 0);
    lv_textarea_set_one_line(_inputArea, false);
    lv_obj_add_event_cb(_inputArea, input_focus_cb, LV_EVENT_FOCUSED, this);
    lv_obj_add_event_cb(_inputArea, input_defocus_cb, LV_EVENT_DEFOCUSED, this);
    lv_obj_add_flag(_inputArea, LV_OBJ_FLAG_HIDDEN);
    
    Serial.println("[ChatApp] createUI done");
    
    return true;
}

void ChatApp::destroyUI() {
    Serial.println("[ChatApp] destroyUI");
    if (_keyboard) {
        lv_obj_del(_keyboard);
        _keyboard = nullptr;
    }
    if (_inputArea) {
        lv_obj_del(_inputArea);
        _inputArea = nullptr;
    }
    if (_floatBtn) {
        lv_obj_del(_floatBtn);
        _floatBtn = nullptr;
    }
    _blankScreen = nullptr;
}

void ChatApp::onUpdate() {
}

void ChatApp::onFloatBtnClick() {
    Serial.println("[ChatApp] onFloatBtnClick");
    toggleInputPanel();
}

void ChatApp::showInputPanel() {
    Serial.println("[ChatApp] showInputPanel");
    if (_inputArea) {
        lv_obj_clear_flag(_inputArea, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(_inputArea);
        lv_obj_move_foreground(_floatBtn);
        _inputPanelVisible = true;
    }
}

void ChatApp::hideInputPanel() {
    Serial.println("[ChatApp] hideInputPanel");
    if (_keyboard) {
        lv_obj_del(_keyboard);
        _keyboard = nullptr;
    }
    if (_inputArea) {
        lv_obj_add_flag(_inputArea, LV_OBJ_FLAG_HIDDEN);
    }
    _inputPanelVisible = false;
}

void ChatApp::toggleInputPanel() {
    Serial.printf("[ChatApp] toggleInputPanel, visible=%d\n", _inputPanelVisible);
    if (_inputPanelVisible) {
        hideInputPanel();
    } else {
        showInputPanel();
    }
}

void ChatApp::float_btn_cb(lv_event_t* e) {
    Serial.println("[ChatApp] float_btn_cb");
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app) {
        app->onFloatBtnClick();
    }
}

void ChatApp::input_focus_cb(lv_event_t* e) {
    Serial.println("[ChatApp] input_focus_cb - show keyboard");
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app && !app->_keyboard && app->_inputArea) {
        app->_keyboard = lv_keyboard_create(app->_screen);
        lv_obj_set_size(app->_keyboard, 240, 100);
        lv_obj_set_pos(app->_keyboard, 0, 0);
        lv_keyboard_set_textarea(app->_keyboard, app->_inputArea);
        lv_keyboard_set_mode(app->_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        
        lv_obj_set_pos(app->_inputArea, 5, 105);
        lv_obj_move_foreground(app->_inputArea);
        lv_obj_move_foreground(app->_keyboard);
        lv_obj_move_foreground(app->_inputArea);
        
        Serial.println("[ChatApp] keyboard at y=0, input at y=105");
    }
}

void ChatApp::input_defocus_cb(lv_event_t* e) {
    Serial.println("[ChatApp] input_defocus_cb - hide keyboard");
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app && app->_keyboard) {
        lv_obj_del(app->_keyboard);
        app->_keyboard = nullptr;
        
        if (app->_inputArea) {
            lv_obj_align(app->_inputArea, LV_ALIGN_BOTTOM_MID, 0, -5);
        }
    }
}

app_info_t ChatApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "Chat", APP_NAME_MAX_LEN - 1);
    strncpy(info.icon, LV_SYMBOL_EDIT, sizeof(info.icon) - 1);
    info.type = APP_TYPE_USER;
    info.enabled = true;
    return info;
}

BaseApp* createChatApp() {
    return new ChatApp();
}
