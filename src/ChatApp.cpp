#include "ChatApp.h"
#include "Storage.h"
#include "FileExplorerApp.h"
#include "LvZhFont.h"
#include "BSP.h"
#include <SD.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

static void onOpenChatCallback(void* user_data) {
    ChatApp* app = (ChatApp*)user_data;
    if (app) app->onOpenChat();
}

static void onNewChatCallback(void* user_data) {
    ChatApp* app = (ChatApp*)user_data;
    if (app) app->onNewChat();
}

static void onModelSelectCallback(void* user_data) {
    ChatApp* app = (ChatApp*)user_data;
    if (app) app->onModelSelect();
}

static void onPromptSelectCallback(void* user_data) {
    ChatApp* app = (ChatApp*)user_data;
    if (app) app->onPromptSelect();
}

ChatApp::ChatApp() : BaseApp("Chat") {
    _blankScreen = nullptr;
    _floatBtn = nullptr;
    _inputArea = nullptr;
    _keyboard = nullptr;
    _modeBtn = nullptr;
    _msgContainer = nullptr;
    _btnOpenChat = nullptr;
    _btnNewChat = nullptr;
    _btnModel = nullptr;
    _btnPrompt = nullptr;
    _modelSelector = nullptr;
    _inputPanelVisible = false;
    _doublePinyinMode = false;
    _sdCardAvailable = false;
    _dataFolderReady = false;
    _isWaitingResponse = false;
    _dpBufferLen = 0;
    _dpBuffer[0] = '\0';
    _preModeText[0] = '\0';
    _currentChatPath[0] = '\0';
    _nextChatIndex = 1;
    _msgHead = nullptr;
    _msgTail = nullptr;
    _msgCount = 0;
    _selectedModelIndex = 0;
    _netTaskHandle = nullptr;
    _pendingMessage[0] = '\0';
    _responseContent[0] = '\0';
    _responseReady = false;
    _systemPrompt[0] = '\0';
    _promptPath[0] = '\0';
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
    lv_obj_set_size(_msgContainer, LV_PCT(100), BSP_DISPLAY_HEIGHT);
    lv_obj_set_pos(_msgContainer, 0, 0);
    lv_obj_set_style_bg_opa(_msgContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_msgContainer, 0, 0);
    lv_obj_set_style_pad_all(_msgContainer, 5, 0);
    lv_obj_set_flex_flow(_msgContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(_msgContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(_msgContainer, LV_DIR_HOR);
    lv_obj_set_scrollbar_mode(_msgContainer, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_right(_msgContainer, 50, 0);
    lv_obj_clear_flag(_msgContainer, LV_OBJ_FLAG_SCROLL_MOMENTUM);
    lv_obj_clear_flag(_msgContainer, LV_OBJ_FLAG_SCROLL_ELASTIC);
    lv_obj_set_scroll_snap_x(_msgContainer, LV_SCROLL_SNAP_NONE);
    
    setupSidebarButtons();
    
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

bool ChatApp::onResume() {
    Serial.println("[ChatApp] onResume");
    
    if (!BaseApp::onResume()) {
        Serial.println("[ChatApp] BaseApp::onResume failed");
        return false;
    }
    
    bsp_set_touch_fps_optimize(true);
    
    checkPendingFile();
    checkPendingPromptFile();
    
    return true;
}

void ChatApp::onPause() {
    Serial.println("[ChatApp] onPause");
    
    bsp_set_touch_fps_optimize(false);
    
    BaseApp::onPause();
}

void ChatApp::saveState() {
    if (!_dataFolderReady) return;
    
    File stateFile = SD.open("/ChatApp/.state", FILE_WRITE);
    if (!stateFile) {
        Serial.println("[ChatApp] Failed to save state");
        return;
    }
    
    stateFile.printf("chat_path=%s\n", _currentChatPath);
    stateFile.printf("input_text=%s\n", _inputArea ? lv_textarea_get_text(_inputArea) : "");
    stateFile.printf("model_index=%d\n", _selectedModelIndex);
    stateFile.printf("prompt_path=%s\n", _promptPath);
    
    stateFile.close();
    Serial.println("[ChatApp] State saved");
}

bool ChatApp::loadState() {
    if (!_sdCardAvailable) return false;
    
    if (!SD.exists("/ChatApp/.state")) {
        Serial.println("[ChatApp] No state file found");
        return false;
    }
    
    File stateFile = SD.open("/ChatApp/.state");
    if (!stateFile) {
        return false;
    }
    
    char chatPath[CHAT_PATH_MAX_LEN] = "";
    char inputText[CHAT_INPUT_MAX_LEN] = "";
    char promptPath[CHAT_PATH_MAX_LEN] = "";
    int modelIndex = 0;
    
    while (stateFile.available()) {
        String line = stateFile.readStringUntil('\n');
        line.trim();
        
        if (line.startsWith("chat_path=")) {
            strncpy(chatPath, line.c_str() + 10, CHAT_PATH_MAX_LEN - 1);
        } else if (line.startsWith("input_text=")) {
            strncpy(inputText, line.c_str() + 11, CHAT_INPUT_MAX_LEN - 1);
        } else if (line.startsWith("model_index=")) {
            modelIndex = atoi(line.c_str() + 12);
            if (modelIndex < 0 || modelIndex >= (int)AI_MODEL_COUNT) {
                modelIndex = 0;
            }
        } else if (line.startsWith("prompt_path=")) {
            strncpy(promptPath, line.c_str() + 12, CHAT_PATH_MAX_LEN - 1);
        }
    }
    
    stateFile.close();
    
    _selectedModelIndex = modelIndex;
    
    if (promptPath[0] != '\0' && SD.exists(promptPath)) {
        loadPromptFromFile(promptPath);
        Serial.printf("[ChatApp] Restored prompt: %s\n", promptPath);
    }
    
    if (chatPath[0] != '\0' && SD.exists(chatPath)) {
        loadChatFromFile(chatPath);
        Serial.printf("[ChatApp] Restored chat: %s\n", chatPath);
    }
    
    if (inputText[0] != '\0' && _inputArea) {
        lv_textarea_set_text(_inputArea, inputText);
        Serial.printf("[ChatApp] Restored input: %s\n", inputText);
    }
    
    return true;
}

void ChatApp::checkPendingFile() {
    if (!_sdCardAvailable) return;
    
    if (!SD.exists("/ChatApp/.pending_file")) {
        return;
    }
    
    File pendingFile = SD.open("/ChatApp/.pending_file");
    if (!pendingFile) return;
    
    String path = pendingFile.readStringUntil('\n');
    path.trim();
    pendingFile.close();
    
    SD.remove("/ChatApp/.pending_file");
    
    if (path.length() > 0) {
        processPendingFile(path.c_str());
    }
}

void ChatApp::processPendingFile(const char* path) {
    Serial.printf("[ChatApp] Processing pending file: %s\n", path);
    
    if (path[0] == 'S' && path[1] == ':') {
        loadChatFromFile(path + 2);
    } else {
        loadChatFromFile(path);
    }
}

void ChatApp::checkPendingPromptFile() {
    if (!_sdCardAvailable) return;
    
    if (!SD.exists("/ChatApp/.pending_prompt")) {
        return;
    }
    
    File pendingFile = SD.open("/ChatApp/.pending_prompt");
    if (!pendingFile) return;
    
    String path = pendingFile.readStringUntil('\n');
    path.trim();
    pendingFile.close();
    
    SD.remove("/ChatApp/.pending_prompt");
    
    if (path.length() > 0) {
        loadPromptFromFile(path.c_str());
    }
}

void ChatApp::loadPromptFromFile(const char* path) {
    Serial.printf("[ChatApp] Loading prompt from: %s\n", path);
    
    const char* actualPath = path;
    if (path[0] == 'S' && path[1] == ':') {
        actualPath = path + 2;
    }
    
    if (!SD.exists(actualPath)) {
        Serial.printf("[ChatApp] Prompt file not found: %s\n", actualPath);
        return;
    }
    
    File promptFile = SD.open(actualPath);
    if (!promptFile) {
        Serial.println("[ChatApp] Failed to open prompt file");
        return;
    }
    
    String content = promptFile.readString();
    promptFile.close();
    
    content.trim();
    
    strncpy(_systemPrompt, content.c_str(), CHAT_PROMPT_MAX_LEN - 1);
    _systemPrompt[CHAT_PROMPT_MAX_LEN - 1] = '\0';
    
    strncpy(_promptPath, actualPath, CHAT_PATH_MAX_LEN - 1);
    _promptPath[CHAT_PATH_MAX_LEN - 1] = '\0';
    
    Serial.printf("[ChatApp] Loaded prompt (%d bytes): %.50s...\n", 
        strlen(_systemPrompt), _systemPrompt);
}

void ChatApp::destroyUI() {
    Serial.println("[ChatApp] destroyUI");
    
    saveState();
    
    clearSidebarButtons();
    
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

void ChatApp::setupSidebarButtons() {
    GlobalUI& ui = GlobalUI::getInstance();
    
    _btnOpenChat = ui.addSidebarButton(LV_SYMBOL_LIST, onOpenChatCallback, this);
    _btnNewChat = ui.addSidebarButton(LV_SYMBOL_EDIT, onNewChatCallback, this);
    
    static char modelSymbol[2] = "D";
    modelSymbol[0] = AI_MODELS[_selectedModelIndex].name[0];
    _btnModel = ui.addSidebarButton(modelSymbol, onModelSelectCallback, this);
    
    _btnPrompt = ui.addSidebarButton(LV_SYMBOL_FILE, onPromptSelectCallback, this);
    
    Serial.println("[ChatApp] Sidebar buttons added");
}

void ChatApp::clearSidebarButtons() {
    GlobalUI& ui = GlobalUI::getInstance();
    
    ui.removeSidebarButton(_btnOpenChat);
    ui.removeSidebarButton(_btnNewChat);
    ui.removeSidebarButton(_btnModel);
    ui.removeSidebarButton(_btnPrompt);
    
    _btnOpenChat = nullptr;
    _btnNewChat = nullptr;
    _btnModel = nullptr;
    _btnPrompt = nullptr;
    
    hideModelSelector();
    
    Serial.println("[ChatApp] Sidebar buttons cleared");
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

void ChatApp::appendMessageToFile(const char* text, bool isSent) {
    if (!_dataFolderReady || !text) return;
    
    if (_currentChatPath[0] == '\0') {
        snprintf(_currentChatPath, sizeof(_currentChatPath), "/ChatApp/chats/%d.txt", _nextChatIndex++);
    }
    
    File file = SD.open(_currentChatPath, FILE_APPEND);
    if (!file) {
        Serial.printf("[ChatApp] Failed to open file for appending: %s\n", _currentChatPath);
        return;
    }
    
    const char* role = isSent ? "user" : "order";
    file.printf("[%s] ", role);
    
    for (const char* p = text; *p; p++) {
        if (*p == '\n') {
            file.print("\\n");
        } else if (*p == '\r') {
            // skip
        } else {
            file.write(*p);
        }
    }
    file.println();
    file.close();
    
    Serial.printf("[ChatApp] Appended message to %s\n", _currentChatPath);
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
        int prefixLen = 0;
        
        if (line.startsWith("[user] ")) {
            isSent = true;
            prefixLen = 7;
        } else if (line.startsWith("[order] ")) {
            isSent = false;
            prefixLen = 8;
        } else {
            continue;
        }
        
        String content = line.substring(prefixLen);
        content.replace("\\n", "\n");
        
        addMessageToList(content.c_str(), isSent);
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
    if (!text) return nullptr;
    
    int textLen = strlen(text);
    if (textLen == 0) return nullptr;
    
    lv_obj_t* bubble = lv_obj_create(_msgContainer);
    lv_obj_set_size(bubble, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(bubble, bgColor, 0);
    lv_obj_set_style_bg_opa(bubble, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bubble, 0, 0);
    lv_obj_set_style_radius(bubble, 0, 0);
    lv_obj_set_style_pad_all(bubble, 8, 0);
    
    lv_obj_t* label = lv_label_create(bubble);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_make(0x20, 0x20, 0x20), 0);
    
    if (LvZhFontMgr.isInitialized()) {
        lv_obj_set_style_text_font(label, LvZhFontMgr.getFont(), 0);
    } else {
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    }
    
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(label, LV_SIZE_CONTENT);
    
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
    
    if (_dataFolderReady) {
        appendMessageToFile(text, isSent);
    }
    
    Serial.printf("[ChatApp] Add message: '%s' (sent=%d)\n", text, isSent);
}

void ChatApp::sendMessage() {
    if (!_inputArea) return;
    
    const char* text = lv_textarea_get_text(_inputArea);
    if (text && strlen(text) > 0) {
        char messageCopy[CHAT_INPUT_MAX_LEN];
        strncpy(messageCopy, text, CHAT_INPUT_MAX_LEN - 1);
        messageCopy[CHAT_INPUT_MAX_LEN - 1] = '\0';
        
        addMessage(messageCopy, true);
        lv_textarea_set_text(_inputArea, "");
        
        sendAIRequestAsync(messageCopy);
    }
}

void ChatApp::onOpenChat() {
    Serial.println("[ChatApp] onOpenChat - request file explorer");
    
    if (!_sdCardAvailable) {
        Serial.println("[ChatApp] SD card not available");
        return;
    }
    
    FileExplorerApp::_explorerMode = MODE_SELECT_FILE;
    strncpy(FileExplorerApp::selectStartPath, "S:/ChatApp/chats", MAX_PATH_LENGTH - 1);
    Serial.printf("[ChatApp] Set select mode, path: %s\n", FileExplorerApp::selectStartPath);
    
    AppMgr.switchToApp("FileExplorer");
}

void ChatApp::onNewChat() {
    Serial.println("[ChatApp] onNewChat");
    clearMessages();
    _currentChatPath[0] = '\0';
}

void ChatApp::onPromptSelect() {
    Serial.println("[ChatApp] onPromptSelect - request prompt file");
    
    if (!_sdCardAvailable) {
        Serial.println("[ChatApp] SD card not available");
        return;
    }
    
    FileExplorerApp::_explorerMode = MODE_SELECT_FILE;
    strncpy(FileExplorerApp::selectStartPath, "S:/ChatApp/prompts", MAX_PATH_LENGTH - 1);
    Serial.printf("[ChatApp] Set prompt select mode, path: %s\n", FileExplorerApp::selectStartPath);
    
    AppMgr.switchToApp("FileExplorer");
}

void ChatApp::onFileSelected(const char* path) {
    Serial.printf("[ChatApp] onFileSelected: %s\n", path);
    
    if (path && strlen(path) > 0) {
        loadChatFromFile(path);
    }
}

void ChatApp::onUpdate() {
    static uint32_t lastCheck = 0;
    if (_isWaitingResponse && millis() - lastCheck > 2000) {
        Serial.printf("[ChatApp] onUpdate: waiting=%d, responseReady=%d\n", 
            _isWaitingResponse, _responseReady);
        lastCheck = millis();
    }
    
    if (_responseReady) {
        Serial.printf("[ChatApp] onUpdate: response ready, len=%d\n", strlen(_responseContent));
        _responseReady = false;
        _isWaitingResponse = false;
        
        if (_responseContent[0] != '\0') {
            addMessage(_responseContent, false);
            Serial.printf("[ChatApp] AI response displayed: %d bytes\n", strlen(_responseContent));
        }
        
        _responseContent[0] = '\0';
    }
}

void ChatApp::onFloatBtnClick() {
    Serial.println("[ChatApp] onFloatBtnClick");
    toggleInputPanel();
}

void ChatApp::showInputPanel() {
    Serial.println("[ChatApp] showInputPanel");
    if (_inputArea) {
        if (_msgContainer) {
            lv_obj_add_flag(_msgContainer, LV_OBJ_FLAG_HIDDEN);
        }
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
    if (_msgContainer) {
        lv_obj_clear_flag(_msgContainer, LV_OBJ_FLAG_HIDDEN);
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
            
            if (_dpBufferLen % 2 == 0 && _dpBufferLen > 0) {
                strcat(tempBuffer, " ");
                strncpy(_preModeText, tempBuffer, CHAT_INPUT_MAX_LEN - 1);
                _preModeText[CHAT_INPUT_MAX_LEN - 1] = '\0';
                _dpBufferLen = 0;
                _dpBuffer[0] = '\0';
            }
            
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
                    
                    if (app->_dpBufferLen % 2 == 0 && app->_dpBufferLen > 0) {
                        strcat(tempBuffer, " ");
                        strncpy(app->_preModeText, tempBuffer, CHAT_INPUT_MAX_LEN - 1);
                        app->_preModeText[CHAT_INPUT_MAX_LEN - 1] = '\0';
                        app->_dpBufferLen = 0;
                        app->_dpBuffer[0] = '\0';
                    }
                    
                    lv_textarea_set_text(app->_inputArea, tempBuffer);
                }
            }
        }
    }
    else if (code == LV_EVENT_READY) {
        Serial.println("[ChatApp] LV_EVENT_READY - OK button clicked");
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

void ChatApp::onModelSelect() {
    Serial.println("[ChatApp] onModelSelect");
    if (_modelSelector) {
        hideModelSelector();
    } else {
        showModelSelector();
    }
}

void ChatApp::showModelSelector() {
    if (_modelSelector) return;
    
    _modelSelector = lv_obj_create(lv_layer_top());
    lv_obj_set_size(_modelSelector, 120, 100);
    lv_obj_align(_modelSelector, LV_ALIGN_TOP_LEFT, 55, 130);
    lv_obj_set_style_bg_color(_modelSelector, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_border_width(_modelSelector, 1, 0);
    lv_obj_set_style_border_color(_modelSelector, lv_color_make(0x60, 0x60, 0x60), 0);
    lv_obj_set_style_radius(_modelSelector, 5, 0);
    lv_obj_set_style_pad_all(_modelSelector, 5, 0);
    lv_obj_set_flex_flow(_modelSelector, LV_FLEX_FLOW_COLUMN);
    
    for (size_t i = 0; i < AI_MODEL_COUNT; i++) {
        lv_obj_t* btn = lv_btn_create(_modelSelector);
        lv_obj_set_size(btn, LV_PCT(100), 25);
        lv_obj_set_style_bg_color(btn, i == (size_t)_selectedModelIndex ? 
            lv_color_make(0x00, 0x80, 0xC0) : lv_color_make(0x50, 0x50, 0x50), 0);
        lv_obj_add_event_cb(btn, model_selector_cb, LV_EVENT_CLICKED, this);
        
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, AI_MODELS[i].name);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        lv_obj_center(label);
    }
    
    Serial.println("[ChatApp] Model selector shown");
}

void ChatApp::hideModelSelector() {
    if (_modelSelector) {
        lv_obj_del(_modelSelector);
        _modelSelector = nullptr;
        Serial.println("[ChatApp] Model selector hidden");
    }
}

void ChatApp::selectModel(int index) {
    if (index < 0 || index >= (int)AI_MODEL_COUNT) return;
    
    _selectedModelIndex = index;
    updateModelButton();
    hideModelSelector();
    Serial.printf("[ChatApp] Selected model: %s\n", AI_MODELS[index].name);
}

void ChatApp::updateModelButton() {
    if (_btnModel) {
        lv_obj_t* label = lv_obj_get_child(_btnModel, 0);
        if (label) {
            char text[2] = {AI_MODELS[_selectedModelIndex].name[0], '\0'};
            lv_label_set_text(label, text);
        }
    }
}

void ChatApp::model_selector_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    lv_obj_t* btn = (lv_obj_t*)lv_event_get_target(e);
    
    if (!app || !btn) return;
    
    lv_obj_t* parent = lv_obj_get_parent(btn);
    int index = 0;
    lv_obj_t* child = lv_obj_get_child(parent, 0);
    while (child && child != btn) {
        child = lv_obj_get_child(parent, ++index);
    }
    
    if (child == btn) {
        app->selectModel(index);
    }
}

bool ChatApp::checkNetworkConnection() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[ChatApp] WiFi not connected");
        return false;
    }
    return true;
}

void ChatApp::sendAIRequestAsync(const char* userMessage) {
    Serial.printf("[ChatApp] sendAIRequestAsync: '%s', waiting=%d\n", userMessage, _isWaitingResponse);
    
    if (!checkNetworkConnection()) {
        _isWaitingResponse = false;
        addMessage("Error: No network", false);
        return;
    }
    
    if (_isWaitingResponse) {
        Serial.println("[ChatApp] Already waiting");
        return;
    }
    
    _isWaitingResponse = true;
    strncpy(_pendingMessage, userMessage, CHAT_INPUT_MAX_LEN - 1);
    _pendingMessage[CHAT_INPUT_MAX_LEN - 1] = '\0';
    _responseReady = false;
    _responseContent[0] = '\0';
    
    Serial.println("[ChatApp] Creating network task...");
    
    BaseType_t ret = xTaskCreatePinnedToCore(
        networkTaskEntry,
        "ChatNetTask",
        CHAT_NET_TASK_STACK,
        this,
        CHAT_NET_TASK_PRIORITY,
        &_netTaskHandle,
        0
    );
    
    if (ret != pdPASS) {
        Serial.println("[ChatApp] Failed to create network task");
        _isWaitingResponse = false;
        addMessage("Error: Task create failed", false);
        _netTaskHandle = nullptr;
    } else {
        Serial.println("[ChatApp] Network task started on Core 0");
    }
}

void ChatApp::networkTaskEntry(void* arg) {
    ChatApp* app = (ChatApp*)arg;
    
    Serial.println("[ChatNet] Task running");
    
    bool success = app->performAIRequest(app->_pendingMessage);
    
    if (!success) {
        strcpy(app->_responseContent, "Error: Request failed");
        app->_responseReady = true;
    }
    
    app->_netTaskHandle = nullptr;
    
    Serial.println("[ChatNet] Task done");
    vTaskDelete(NULL);
}

bool ChatApp::performAIRequest(const char* userMessage) {
    const ai_model_config_t& model = AI_MODELS[_selectedModelIndex];
    Serial.printf("[ChatNet] Request to %s\n", model.name);
    
    File tempFile = SD.open(CHAT_TEMP_FILE, FILE_WRITE);
    if (!tempFile) {
        Serial.println("[ChatNet] Failed to create temp file");
        return false;
    }
    
    WiFiClientSecure* client = new WiFiClientSecure();
    client->setInsecure();
    client->setTimeout(30000);
    
    char host[64];
    int port = 443;
    const char* urlStart = strstr(model.endpoint, "https://");
    if (urlStart) {
        urlStart += 8;
    } else {
        urlStart = model.endpoint;
    }
    const char* path = strchr(urlStart, '/');
    int hostLen = path ? (path - urlStart) : strlen(urlStart);
    if (hostLen > 63) hostLen = 63;
    strncpy(host, urlStart, hostLen);
    host[hostLen] = '\0';
    const char* uri = path ? path : "/";
    
    Serial.printf("[ChatNet] Connecting to %s:%d%s\n", host, port, uri);
    
    if (!client->connect(host, port)) {
        Serial.println("[ChatNet] Connection failed");
        tempFile.close();
        SD.remove(CHAT_TEMP_FILE);
        delete client;
        return false;
    }
    
    char body[CHAT_INPUT_MAX_LEN + CHAT_PROMPT_MAX_LEN + 256];
    int bodyLen;
    
    if (_systemPrompt[0] != '\0') {
        bodyLen = snprintf(body, sizeof(body), 
            "{\"model\":\"%s\",\"messages\":[{\"role\":\"system\",\"content\":\"%s\"},{\"role\":\"user\",\"content\":\"%s\"}],\"stream\":true}",
            model.model, _systemPrompt, userMessage);
        Serial.printf("[ChatNet] Request with system prompt (%d bytes)\n", strlen(_systemPrompt));
    } else {
        bodyLen = snprintf(body, sizeof(body), 
            "{\"model\":\"%s\",\"messages\":[{\"role\":\"user\",\"content\":\"%s\"}],\"stream\":true}",
            model.model, userMessage);
    }
    
    client->printf("POST %s HTTP/1.1\r\n", uri);
    client->printf("Host: %s\r\n", host);
    client->print("Content-Type: application/json\r\n");
    client->printf("Authorization: Bearer %s\r\n", model.apiKey);
    client->printf("Content-Length: %d\r\n", bodyLen);
    client->print("Connection: close\r\n");
    client->print("\r\n");
    client->print(body);
    
    Serial.println("[ChatNet] Request sent, receiving...");
    
    static char lineBuf[1024];
    
    int linePos = 0;
    bool inBody = false;
    uint32_t timeout = millis();
    uint32_t lastPrint = 0;
    int totalBytes = 0;
    
    while (millis() - timeout < 30000) {
        if (client->available()) {
            timeout = millis();
            char c = client->read();
            totalBytes++;
            
            if (!inBody) {
                if (c == '\n') {
                    lineBuf[linePos] = '\0';
                    if (linePos == 0 || (linePos == 1 && lineBuf[0] == '\r')) {
                        inBody = true;
                        Serial.println("[ChatNet] Headers received, entering body");
                    }
                    linePos = 0;
                } else if (linePos < 1023) {
                    lineBuf[linePos++] = c;
                }
            } else {
                if (c == '\n') {
                    lineBuf[linePos] = '\0';
                    if (linePos > 0 && lineBuf[0] != '\r') {
                        tempFile.println(lineBuf);
                    }
                    linePos = 0;
                } else if (linePos < 1023) {
                    lineBuf[linePos++] = c;
                }
            }
        } else if (!client->connected()) {
            Serial.println("[ChatNet] Server disconnected");
            break;
        } else {
            if (millis() - lastPrint > 5000) {
                Serial.printf("[ChatNet] Waiting... bytes=%d\n", totalBytes);
                lastPrint = millis();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    Serial.printf("[ChatNet] Received %d bytes\n", totalBytes);
    
    client->stop();
    delete client;
    tempFile.close();
    
    Serial.println("[ChatNet] Response saved, parsing...");
    
    processAIResponse();
    return true;
}

void ChatApp::processAIResponse() {
    if (!SD.exists(CHAT_TEMP_FILE)) {
        Serial.println("[ChatNet] Temp file not found");
        strcpy(_responseContent, "Error: No response");
        _responseReady = true;
        return;
    }
    
    File tempFile = SD.open(CHAT_TEMP_FILE);
    if (!tempFile) {
        Serial.println("[ChatNet] Failed to open temp file");
        strcpy(_responseContent, "Error: Read failed");
        _responseReady = true;
        return;
    }
    
    Serial.printf("[ChatNet] Temp file size: %d bytes\n", tempFile.size());
    
    _responseContent[0] = '\0';
    int contentLen = 0;
    
    static char parseBuf[1024];
    
    int linePos = 0;
    int lineCount = 0;
    
    while (tempFile.available() && contentLen < CHAT_MSG_MAX_LEN - 64) {
        char c = tempFile.read();
        if (c == '\n') {
            parseBuf[linePos] = '\0';
            lineCount++;
            
            if (lineCount <= 5) {
                Serial.printf("[ChatNet] Line %d: '%s'\n", lineCount, parseBuf);
            }
            
            int lineLen = strlen(parseBuf);
            if (lineLen > 0 && parseBuf[lineLen-1] == '\r') {
                parseBuf[--lineLen] = '\0';
            }
            
            bool isChunkSize = true;
            for (int i = 0; i < lineLen; i++) {
                char ch = parseBuf[i];
                if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) {
                    isChunkSize = false;
                    break;
                }
            }
            if (isChunkSize && lineLen > 0 && lineLen <= 4) {
                linePos = 0;
                continue;
            }
            
            if (strncmp(parseBuf, "data: ", 6) == 0) {
                const char* data = parseBuf + 6;
                if (strcmp(data, "[DONE]") == 0) {
                    Serial.println("[ChatNet] Got [DONE]");
                    break;
                }
                
                char content[128];
                parseSSELine(data, content, sizeof(content));
                int cLen = strlen(content);
                if (cLen > 0 && contentLen + cLen < CHAT_MSG_MAX_LEN - 1) {
                    strcat(_responseContent, content);
                    contentLen += cLen;
                }
            }
            
            linePos = 0;
        } else if (linePos < 1023) {
            parseBuf[linePos++] = c;
        }
    }
    
    tempFile.close();
    SD.remove(CHAT_TEMP_FILE);
    
    _responseReady = true;
    Serial.printf("[ChatNet] Parsed response: %d bytes from %d lines\n", contentLen, lineCount);
}

void ChatApp::parseSSELine(const char* line, char* content, int maxLen) {
    content[0] = '\0';
    
    const char* contentStart = strstr(line, "\"reasoning_content\":\"");
    if (contentStart) {
        contentStart += 21;
    } else {
        contentStart = strstr(line, "\"content\":\"");
        if (!contentStart) return;
        contentStart += 11;
    }
    
    int i = 0;
    while (*contentStart && *contentStart != '"' && i < maxLen - 1) {
        if (*contentStart == '\\' && *(contentStart + 1) == 'n') {
            content[i++] = '\n';
            contentStart += 2;
        } else if (*contentStart == '\\' && *(contentStart + 1) == '"') {
            content[i++] = '"';
            contentStart += 2;
        } else if (*contentStart == '\\' && *(contentStart + 1) == '\\') {
            content[i++] = '\\';
            contentStart += 2;
        } else {
            content[i++] = *contentStart++;
        }
    }
    content[i] = '\0';
}

BaseApp* createChatApp() {
    return new ChatApp();
}
