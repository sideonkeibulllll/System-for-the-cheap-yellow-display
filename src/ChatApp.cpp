#include "ChatApp.h"
#include "Storage.h"
#include "FileExplorerApp.h"
#include <SD.h>

ChatApp::ChatApp() : BaseApp("Chat") {
    _blankScreen = nullptr;
    _floatBtn = nullptr;
    _inputArea = nullptr;
    _keyboard = nullptr;
    _modeBtn = nullptr;
    _msgContainer = nullptr;
    _sidebar = nullptr;
    _btnOpenChat = nullptr;
    _btnNewChat = nullptr;
    _btnPlaceholder = nullptr;
    _inputPanelVisible = false;
    _doublePinyinMode = false;
    _sdCardAvailable = false;
    _dataFolderReady = false;
    _dpBufferLen = 0;
    _dpBuffer[0] = '\0';
    _preModeText[0] = '\0';
    _currentChatPath[0] = '\0';
    _nextChatIndex = 1;
    _msgHead = nullptr;
    _msgTail = nullptr;
    _msgCount = 0;
}

ChatApp::~ChatApp() {
    clearMessages();
}

bool ChatApp::createUI() {
    Serial.println("[ChatApp] createUI start");
    
    _sdCardAvailable = Storage.isSDReady();
    
    if (_sdCardAvailable) {
        initDataFolder();
    }
    
    _blankScreen = lv_obj_create(_screen);
    lv_obj_set_size(_blankScreen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(_blankScreen, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(_blankScreen, 0, 0);
    lv_obj_set_style_pad_all(_blankScreen, 0, 0);
    
    _msgContainer = lv_obj_create(_blankScreen);
    lv_obj_set_size(_msgContainer, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_pos(_msgContainer, 0, 0);
    lv_obj_set_style_bg_opa(_msgContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_msgContainer, 0, 0);
    lv_obj_set_style_pad_all(_msgContainer, 5, 0);
    lv_obj_set_flex_flow(_msgContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_msgContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    createSidebar();
    
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
    
    if (_dataFolderReady && _msgCount > 0) {
        saveChatToFile();
    }
    
    destroySidebar();
    
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
    _modeBtn = nullptr;
    _msgContainer = nullptr;
}

void ChatApp::createSidebar() {
    if (_sidebar) return;
    
    _sidebar = lv_obj_create(_screen);
    lv_obj_set_size(_sidebar, 50, 140);
    lv_obj_align(_sidebar, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_bg_color(_sidebar, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_bg_opa(_sidebar, LV_OPA_90, 0);
    lv_obj_set_style_border_width(_sidebar, 1, 0);
    lv_obj_set_style_border_color(_sidebar, lv_color_make(0x50, 0x50, 0x50), 0);
    lv_obj_set_style_radius(_sidebar, 8, 0);
    lv_obj_set_style_pad_all(_sidebar, 5, 0);
    lv_obj_set_flex_flow(_sidebar, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(_sidebar, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_move_background(_sidebar);
    
    _btnOpenChat = lv_btn_create(_sidebar);
    lv_obj_set_size(_btnOpenChat, 40, 40);
    lv_obj_set_style_bg_color(_btnOpenChat, lv_color_make(0x00, 0x80, 0x40), 0);
    lv_obj_set_style_radius(_btnOpenChat, 8, 0);
    lv_obj_add_event_cb(_btnOpenChat, sidebar_btn_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* labelOpen = lv_label_create(_btnOpenChat);
    lv_label_set_text(labelOpen, LV_SYMBOL_LIST);
    lv_obj_set_style_text_color(labelOpen, lv_color_white(), 0);
    lv_obj_center(labelOpen);
    
    _btnNewChat = lv_btn_create(_sidebar);
    lv_obj_set_size(_btnNewChat, 40, 40);
    lv_obj_set_style_bg_color(_btnNewChat, lv_color_make(0x40, 0x80, 0x00), 0);
    lv_obj_set_style_radius(_btnNewChat, 8, 0);
    lv_obj_add_event_cb(_btnNewChat, sidebar_btn_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* labelNew = lv_label_create(_btnNewChat);
    lv_label_set_text(labelNew, LV_SYMBOL_EDIT);
    lv_obj_set_style_text_color(labelNew, lv_color_white(), 0);
    lv_obj_center(labelNew);
    
    _btnPlaceholder = lv_btn_create(_sidebar);
    lv_obj_set_size(_btnPlaceholder, 40, 40);
    lv_obj_set_style_bg_color(_btnPlaceholder, lv_color_make(0x50, 0x50, 0x50), 0);
    lv_obj_set_style_radius(_btnPlaceholder, 8, 0);
    lv_obj_add_flag(_btnPlaceholder, LV_OBJ_FLAG_CLICKABLE);
    
    lv_obj_t* labelPlaceholder = lv_label_create(_btnPlaceholder);
    lv_label_set_text(labelPlaceholder, "-");
    lv_obj_set_style_text_color(labelPlaceholder, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_center(labelPlaceholder);
    
    Serial.println("[ChatApp] Sidebar created");
}

void ChatApp::destroySidebar() {
    if (_sidebar) {
        lv_obj_del(_sidebar);
        _sidebar = nullptr;
        _btnOpenChat = nullptr;
        _btnNewChat = nullptr;
        _btnPlaceholder = nullptr;
    }
}

bool ChatApp::initDataFolder() {
    if (!_sdCardAvailable) return false;
    
    if (!SD.exists("/ChatApp")) {
        if (SD.mkdir("/ChatApp")) {
            Serial.println("[ChatApp] Created /ChatApp folder");
        } else {
            Serial.println("[ChatApp] Failed to create /ChatApp folder");
            return false;
        }
    }
    
    if (!SD.exists("/ChatApp/chats")) {
        if (SD.mkdir("/ChatApp/chats")) {
            Serial.println("[ChatApp] Created /ChatApp/chats folder");
        } else {
            return false;
        }
    }
    
    if (!SD.exists("/ChatApp/prompts")) {
        if (SD.mkdir("/ChatApp/prompts")) {
            Serial.println("[ChatApp] Created /ChatApp/prompts folder");
        } else {
            return false;
        }
    }
    
    _nextChatIndex = getNextChatIndex();
    _dataFolderReady = true;
    Serial.printf("[ChatApp] Data folder ready, next chat index: %d\n", _nextChatIndex);
    
    return true;
}

int ChatApp::getNextChatIndex() {
    if (!_sdCardAvailable) return 1;
    
    int maxIndex = 0;
    File chatsDir = SD.open("/ChatApp/chats");
    if (!chatsDir || !chatsDir.isDirectory()) {
        return 1;
    }
    
    File file = chatsDir.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            const char* name = file.name();
            const char* lastSlash = strrchr(name, '/');
            const char* filename = lastSlash ? lastSlash + 1 : name;
            
            int index = 0;
            if (sscanf(filename, "%d", &index) == 1) {
                if (index > maxIndex) maxIndex = index;
            }
        }
        file = chatsDir.openNextFile();
    }
    chatsDir.close();
    
    return maxIndex + 1;
}

void ChatApp::addMessageToList(const char* text, bool isSent) {
    if (!text) return;
    
    size_t textLen = strlen(text) + 1;
    char* textCopy = (char*)malloc(textLen);
    if (!textCopy) {
        Serial.println("[ChatApp] Failed to allocate text memory");
        return;
    }
    strcpy(textCopy, text);
    
    ChatMessage* msg = (ChatMessage*)malloc(sizeof(ChatMessage));
    if (!msg) {
        Serial.println("[ChatApp] Failed to allocate message memory");
        free(textCopy);
        return;
    }
    
    msg->text = textCopy;
    msg->isSent = isSent;
    msg->next = nullptr;
    
    if (_msgTail) {
        _msgTail->next = msg;
        _msgTail = msg;
    } else {
        _msgHead = msg;
        _msgTail = msg;
    }
    _msgCount++;
}

void ChatApp::saveChatToFile() {
    if (!_dataFolderReady || _msgCount == 0) return;
    
    const char* savePath;
    char defaultPath[CHAT_PATH_MAX_LEN];
    
    if (_currentChatPath[0] == '\0') {
        snprintf(defaultPath, sizeof(defaultPath), "/ChatApp/chats/%d.txt", _nextChatIndex++);
        savePath = defaultPath;
        strncpy(_currentChatPath, defaultPath, CHAT_PATH_MAX_LEN - 1);
    } else {
        savePath = _currentChatPath;
    }
    
    File file = SD.open(savePath, FILE_WRITE);
    if (!file) {
        Serial.printf("[ChatApp] Failed to open file for writing: %s\n", savePath);
        return;
    }
    
    ChatMessage* msg = _msgHead;
    while (msg) {
        const char* role = msg->isSent ? "user" : "order";
        file.printf("[%s] %s\n", role, msg->text);
        msg = msg->next;
    }
    
    file.close();
    Serial.printf("[ChatApp] Saved %d messages to %s\n", _msgCount, savePath);
}

bool ChatApp::loadChatFromFile(const char* path) {
    if (!_sdCardAvailable || !path) return false;
    
    File file = SD.open(path);
    if (!file) {
        Serial.printf("[ChatApp] Failed to open file: %s\n", path);
        return false;
    }
    
    clearMessages();
    strncpy(_currentChatPath, path, CHAT_PATH_MAX_LEN - 1);
    
    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        
        if (line.length() == 0) continue;
        
        bool isSent = false;
        const char* text = nullptr;
        
        if (line.startsWith("[user] ")) {
            isSent = true;
            text = line.c_str() + 7;
        } else if (line.startsWith("[order] ")) {
            isSent = false;
            text = line.c_str() + 8;
        } else {
            continue;
        }
        
        addMessageToList(text, isSent);
    }
    
    file.close();
    refreshMessageDisplay();
    Serial.printf("[ChatApp] Loaded %d messages from %s\n", _msgCount, path);
    
    return true;
}

void ChatApp::clearMessages() {
    ChatMessage* msg = _msgHead;
    while (msg) {
        ChatMessage* next = msg->next;
        if (msg->text) {
            free(msg->text);
        }
        free(msg);
        msg = next;
    }
    
    _msgHead = nullptr;
    _msgTail = nullptr;
    _msgCount = 0;
    _currentChatPath[0] = '\0';
    
    if (_msgContainer) {
        lv_obj_clean(_msgContainer);
    }
}

void ChatApp::refreshMessageDisplay() {
    if (!_msgContainer) return;
    
    lv_obj_clean(_msgContainer);
    
    ChatMessage* msg = _msgHead;
    while (msg) {
        lv_color_t bgColor = msg->isSent ? 
            lv_color_make(0x95, 0xEC, 0x69) : 
            lv_color_make(0xFF, 0xFF, 0xFF);
        
        lv_obj_t* bubble = createMessageBubble(bgColor, msg->text);
        
        if (msg->isSent) {
            lv_obj_set_style_pad_left(bubble, 20, 0);
            lv_obj_set_style_pad_right(bubble, 5, 0);
        } else {
            lv_obj_set_style_pad_left(bubble, 5, 0);
            lv_obj_set_style_pad_right(bubble, 20, 0);
        }
        
        msg = msg->next;
    }
}

lv_obj_t* ChatApp::createMessageBubble(lv_color_t bgColor, const char* text) {
    lv_obj_t* bubble = lv_obj_create(_msgContainer);
    lv_obj_set_width(bubble, 300);
    lv_obj_set_height(bubble, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(bubble, bgColor, 0);
    lv_obj_set_style_bg_opa(bubble, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bubble, 0, 0);
    lv_obj_set_style_radius(bubble, 8, 0);
    lv_obj_set_style_pad_all(bubble, 8, 0);
    
    lv_obj_t* label = lv_label_create(bubble);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, 280);
    
    return bubble;
}

void ChatApp::addMessage(const char* text, bool isSent) {
    if (!text || strlen(text) == 0) return;
    
    addMessageToList(text, isSent);
    
    lv_color_t bgColor = isSent ? lv_color_make(0x95, 0xEC, 0x69) : lv_color_make(0xFF, 0xFF, 0xFF);
    
    lv_obj_t* bubble = createMessageBubble(bgColor, text);
    
    if (isSent) {
        lv_obj_set_style_pad_left(bubble, 20, 0);
        lv_obj_set_style_pad_right(bubble, 5, 0);
    } else {
        lv_obj_set_style_pad_left(bubble, 5, 0);
        lv_obj_set_style_pad_right(bubble, 20, 0);
    }
    
    Serial.printf("[ChatApp] Add message: '%s' (sent=%d)\n", text, isSent);
}

void ChatApp::sendMessage() {
    if (!_inputArea) return;
    
    const char* text = lv_textarea_get_text(_inputArea);
    if (text && strlen(text) > 0) {
        addMessage(text, true);
        lv_textarea_set_text(_inputArea, "");
        
        if (_dataFolderReady) {
            saveChatToFile();
        }
    }
}

void ChatApp::onOpenChat() {
    Serial.println("[ChatApp] onOpenChat - request file explorer");
    
    if (!_sdCardAvailable) {
        Serial.println("[ChatApp] SD card not available");
        return;
    }
    
    FileExplorerApp::selectCallback = [](const char* path) {
        ChatApp* chatApp = (ChatApp*)AppMgr.findApp("Chat");
        if (chatApp) {
            chatApp->onFileSelected(path);
        }
    };
    
    FileExplorerApp::selectStartPath[0] = '\0';
    
    BaseApp* filesApp = AppMgr.findApp("Files");
    if (filesApp) {
        FileExplorerApp* fileExplorer = (FileExplorerApp*)filesApp;
        fileExplorer->setSelectMode(MODE_SELECT_FILE, "S:/ChatApp/chats");
    }
    
    AppMgr.switchToApp("Files");
}

void ChatApp::onNewChat() {
    Serial.println("[ChatApp] onNewChat");
    clearMessages();
    _currentChatPath[0] = '\0';
}

void ChatApp::onFileSelected(const char* path) {
    Serial.printf("[ChatApp] onFileSelected: %s\n", path);
    
    if (path && strlen(path) > 0) {
        loadChatFromFile(path);
    }
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
    
    sendMessage();
    
    if (_keyboard) {
        lv_obj_del(_keyboard);
        _keyboard = nullptr;
    }
    if (_inputArea) {
        lv_obj_add_flag(_inputArea, LV_OBJ_FLAG_HIDDEN);
    }
    _inputPanelVisible = false;
    _modeBtn = nullptr;
    
    if (_doublePinyinMode) {
        flushDpBuffer();
        _doublePinyinMode = false;
    }
}

void ChatApp::toggleInputPanel() {
    Serial.printf("[ChatApp] toggleInputPanel, visible=%d\n", _inputPanelVisible);
    if (_inputPanelVisible) {
        hideInputPanel();
    } else {
        showInputPanel();
    }
}

void ChatApp::toggleDoublePinyinMode() {
    _doublePinyinMode = !_doublePinyinMode;
    Serial.printf("[ChatApp] DoublePinyin mode: %s\n", _doublePinyinMode ? "ON" : "OFF");
    
    if (_doublePinyinMode) {
        if (_inputArea) {
            const char* text = lv_textarea_get_text(_inputArea);
            strncpy(_preModeText, text, CHAT_INPUT_MAX_LEN - 1);
            _preModeText[CHAT_INPUT_MAX_LEN - 1] = '\0';
        }
        _dpBufferLen = 0;
        _dpBuffer[0] = '\0';
        
        if (_keyboard) {
            lv_keyboard_set_textarea(_keyboard, nullptr);
        }
    } else {
        flushDpBuffer();
        
        if (_keyboard && _inputArea) {
            lv_keyboard_set_textarea(_keyboard, _inputArea);
        }
    }
    
    updateModeButtonStyle();
}

void ChatApp::updateModeButtonStyle() {
    if (!_modeBtn) return;
    
    if (_doublePinyinMode) {
        lv_obj_set_style_bg_color(_modeBtn, lv_color_make(0x80, 0x80, 0x80), 0);
        lv_obj_set_style_bg_opa(_modeBtn, LV_OPA_50, 0);
    } else {
        lv_obj_set_style_bg_color(_modeBtn, lv_color_make(0xE0, 0xE0, 0xE0), 0);
        lv_obj_set_style_bg_opa(_modeBtn, LV_OPA_COVER, 0);
    }
}

void ChatApp::flushDpBuffer() {
    if (_dpBufferLen == 0 || !_inputArea) return;
    
    char converted[CHAT_DP_BUFFER_SIZE * 3];
    ZiranmaMapping::convertDoublePinyin(_dpBuffer, converted, sizeof(converted));
    
    Serial.printf("[ChatApp] Flush DP buffer: '%s' -> '%s'\n", _dpBuffer, converted);
    
    const char* currentText = lv_textarea_get_text(_inputArea);
    int preLen = strlen(_preModeText);
    int convertedLen = strlen(converted);
    
    char newText[CHAT_INPUT_MAX_LEN];
    strncpy(newText, _preModeText, CHAT_INPUT_MAX_LEN - 1);
    if (preLen + convertedLen < CHAT_INPUT_MAX_LEN) {
        strcat(newText, converted);
    }
    newText[CHAT_INPUT_MAX_LEN - 1] = '\0';
    
    lv_textarea_set_text(_inputArea, newText);
    
    _dpBufferLen = 0;
    _dpBuffer[0] = '\0';
}

void ChatApp::processDoublePinyinInput(const char* newChars) {
    if (!_doublePinyinMode || !newChars) return;
    
    int addLen = strlen(newChars);
    if (_dpBufferLen + addLen < CHAT_DP_BUFFER_SIZE - 1) {
        strcat(_dpBuffer, newChars);
        _dpBufferLen += addLen;
        
        Serial.printf("[ChatApp] DP buffer: '%s' (len=%d)\n", _dpBuffer, _dpBufferLen);
    }
}

void ChatApp::onKeyboardButtonClick(lv_obj_t* btn) {
    if (!_inputArea) return;
    
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    if (!label) return;
    
    const char* btnText = lv_label_get_text(label);
    if (!btnText) return;
    
    Serial.printf("[ChatApp] onKeyboardButtonClick: '%s', DPmode=%d\n", btnText, _doublePinyinMode);
    
    if (strcmp(btnText, LV_SYMBOL_BACKSPACE) == 0) {
        if (_doublePinyinMode && _dpBufferLen > 0) {
            _dpBuffer[--_dpBufferLen] = '\0';
            Serial.printf("[ChatApp] DP backspace, buffer: '%s'\n", _dpBuffer);
            
            char tempBuffer[CHAT_DP_BUFFER_SIZE * 3];
            strcpy(tempBuffer, _preModeText);
            if (_dpBufferLen > 0) {
                ZiranmaMapping::convertDoublePinyin(_dpBuffer, tempBuffer + strlen(_preModeText), 
                                                    sizeof(tempBuffer) - strlen(_preModeText));
            }
            lv_textarea_set_text(_inputArea, tempBuffer);
        }
        return;
    }
    
    if (strlen(btnText) != 1) return;
    
    char c = btnText[0];
    if (c >= 'a' && c <= 'z') {
        if (_doublePinyinMode) {
            char str[2] = {c, '\0'};
            processDoublePinyinInput(str);
            
            char tempBuffer[CHAT_DP_BUFFER_SIZE * 3];
            strcpy(tempBuffer, _preModeText);
            ZiranmaMapping::convertDoublePinyin(_dpBuffer, tempBuffer + strlen(_preModeText), 
                                                sizeof(tempBuffer) - strlen(_preModeText));
            lv_textarea_set_text(_inputArea, tempBuffer);
        }
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
        lv_obj_set_size(app->_keyboard, 320, 120);
        lv_obj_align(app->_keyboard, LV_ALIGN_TOP_MID, 0, 65);
        
        lv_obj_clear_flag(app->_inputArea, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(app->_inputArea, LV_ALIGN_TOP_MID, 0, 5);
        lv_obj_move_foreground(app->_inputArea);
        lv_obj_move_foreground(app->_keyboard);
        
        lv_keyboard_set_textarea(app->_keyboard, app->_inputArea);
        lv_keyboard_set_mode(app->_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        
        lv_obj_add_event_cb(app->_keyboard, keyboard_event_cb, LV_EVENT_ALL, app);
        
        Serial.println("[ChatApp] keyboard 320x120 at y=65, input at y=5");
    }
}

void ChatApp::input_defocus_cb(lv_event_t* e) {
    Serial.println("[ChatApp] input_defocus_cb - hide keyboard");
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app && app->_keyboard) {
        lv_obj_del(app->_keyboard);
        app->_keyboard = nullptr;
        app->_modeBtn = nullptr;
        
        if (app->_inputArea) {
            lv_obj_align(app->_inputArea, LV_ALIGN_BOTTOM_MID, 0, -5);
        }
    }
}

void ChatApp::keyboard_btn_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    
    if (!app || !btn) return;
    
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    const char* btnText = label ? lv_label_get_text(label) : nullptr;
    
    Serial.printf("[ChatApp] keyboard_btn_cb: text='%s'\n", btnText ? btnText : "NULL");
    
    if (btnText && strcmp(btnText, LV_SYMBOL_KEYBOARD) == 0) {
        Serial.println("[ChatApp] Keyboard mode button clicked!");
        app->toggleDoublePinyinMode();
        return;
    }
    
    app->onKeyboardButtonClick(btn);
}

void ChatApp::keyboard_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    
    if (!app) return;
    
    if (code == LV_EVENT_CANCEL) {
        Serial.println("[ChatApp] LV_EVENT_CANCEL - left-bottom button clicked!");
        app->toggleDoublePinyinMode();
    }
    else if (code == LV_EVENT_VALUE_CHANGED) {
        if (app->_doublePinyinMode) {
            lv_obj_t* kb = lv_event_get_target(e);
            uint16_t btn_id = lv_keyboard_get_selected_btn(kb);
            if (btn_id != LV_BTNMATRIX_BTN_NONE) {
                const char* btnText = lv_keyboard_get_btn_text(kb, btn_id);
                Serial.printf("[ChatApp] VALUE_CHANGED btn_id=%d text='%s'\n", btn_id, btnText ? btnText : "NULL");
                
                if (btnText && strlen(btnText) == 1 && btnText[0] >= 'a' && btnText[0] <= 'z') {
                    lv_event_stop_bubbling(e);
                    
                    char c = btnText[0];
                    char str[2] = {c, '\0'};
                    app->processDoublePinyinInput(str);
                    
                    char tempBuffer[CHAT_DP_BUFFER_SIZE * 3];
                    strcpy(tempBuffer, app->_preModeText);
                    ZiranmaMapping::convertDoublePinyin(app->_dpBuffer, tempBuffer + strlen(app->_preModeText), 
                                                        sizeof(tempBuffer) - strlen(app->_preModeText));
                    lv_textarea_set_text(app->_inputArea, tempBuffer);
                }
            }
        }
    }
    else if (code == LV_EVENT_READY) {
        Serial.println("[ChatApp] LV_EVENT_READY - OK button clicked");
    }
}

void ChatApp::sidebar_btn_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    
    if (!app || !btn) return;
    
    if (btn == app->_btnOpenChat) {
        app->onOpenChat();
    } else if (btn == app->_btnNewChat) {
        app->onNewChat();
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
