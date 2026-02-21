#include "ChatApp.h"
#include "Storage.h"
#include "AppManager.h"
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <SD.h>

ChatApp::ChatApp() : BaseApp("Chat") {
    _sessionCount = 0;
    _activeSession = -1;
    _floatPanelOpen = false;
    _sdAvailable = false;
    _lastSaveTime = 0;
    _needsSave = false;
    _waitingResponse = false;
    _storagePath[0] = '\0';
    
    mainScreen = nullptr;
    chatContainer = nullptr;
    chatList = nullptr;
    floatBtn = nullptr;
    floatPanel = nullptr;
    historyList = nullptr;
    btnNewChat = nullptr;
    inputArea = nullptr;
    btnSend = nullptr;
    btnClose = nullptr;
}

ChatApp::~ChatApp() {
    if (_needsSave) {
        saveHistory();
    }
}

bool ChatApp::createUI() {
    GLM.setApiKey(GLM_API_KEY);
    GLM.begin();
    
    createMainUI();
    createFloatPanel();
    
    checkStorage();
    loadHistory();
    
    if (_sessionCount == 0) {
        createNewSession();
    } else {
        switchSession(0);
    }
    
    return true;
}

void ChatApp::createMainUI() {
    mainScreen = lv_obj_create(_screen);
    lv_obj_set_size(mainScreen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(mainScreen, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_pad_all(mainScreen, 0, 0);
    lv_obj_set_style_border_width(mainScreen, 0, 0);
    
    chatContainer = lv_obj_create(mainScreen);
    lv_obj_set_size(chatContainer, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(chatContainer, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_pad_all(chatContainer, 8, 0);
    lv_obj_set_style_border_width(chatContainer, 0, 0);
    lv_obj_set_scrollbar_mode(chatContainer, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_flex_flow(chatContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(chatContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    
    chatList = lv_obj_create(chatContainer);
    lv_obj_set_size(chatList, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(chatList, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(chatList, 0, 0);
    lv_obj_set_style_border_width(chatList, 0, 0);
    lv_obj_set_flex_flow(chatList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(chatList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    
    floatBtn = lv_btn_create(mainScreen);
    lv_obj_set_size(floatBtn, 56, 56);
    lv_obj_align(floatBtn, LV_ALIGN_BOTTOM_RIGHT, -12, -12);
    lv_obj_set_style_bg_color(floatBtn, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_radius(floatBtn, 28, 0);
    lv_obj_set_style_shadow_width(floatBtn, 8, 0);
    lv_obj_set_style_shadow_color(floatBtn, lv_color_hex(0x000000), 0);
    lv_obj_add_event_cb(floatBtn, float_btn_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* btnIcon = lv_label_create(floatBtn);
    lv_label_set_text(btnIcon, LV_SYMBOL_EDIT);
    lv_obj_set_style_text_color(btnIcon, lv_color_hex(0xffffff), 0);
    lv_obj_center(btnIcon);
}

void ChatApp::createFloatPanel() {
    floatPanel = lv_obj_create(mainScreen);
    lv_obj_set_size(floatPanel, LV_PCT(92), LV_SIZE_CONTENT);
    lv_obj_align(floatPanel, LV_ALIGN_BOTTOM_MID, 0, -76);
    lv_obj_set_style_bg_color(floatPanel, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_radius(floatPanel, 16, 0);
    lv_obj_set_style_shadow_width(floatPanel, 12, 0);
    lv_obj_set_style_shadow_color(floatPanel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(floatPanel, 12, 0);
    lv_obj_set_style_border_width(floatPanel, 1, 0);
    lv_obj_set_style_border_color(floatPanel, lv_color_hex(0x0f3460), 0);
    lv_obj_add_flag(floatPanel, LV_OBJ_FLAG_HIDDEN);
    
    lv_obj_t* headerRow = lv_obj_create(floatPanel);
    lv_obj_set_size(headerRow, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(headerRow, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(headerRow, 0, 0);
    lv_obj_set_style_border_width(headerRow, 0, 0);
    lv_obj_set_flex_flow(headerRow, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(headerRow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    lv_obj_t* title = lv_label_create(headerRow);
    lv_label_set_text(title, "Chat");
    lv_obj_set_style_text_color(title, lv_color_hex(0xe94560), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    
    btnClose = lv_btn_create(headerRow);
    lv_obj_set_size(btnClose, 32, 32);
    lv_obj_set_style_bg_color(btnClose, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_radius(btnClose, 16, 0);
    lv_obj_add_event_cb(btnClose, close_panel_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* closeIcon = lv_label_create(btnClose);
    lv_label_set_text(closeIcon, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_color(closeIcon, lv_color_hex(0xffffff), 0);
    lv_obj_center(closeIcon);
    
    historyList = lv_list_create(floatPanel);
    lv_obj_set_size(historyList, LV_PCT(100), 80);
    lv_obj_set_style_bg_color(historyList, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_radius(historyList, 8, 0);
    lv_obj_set_style_border_width(historyList, 0, 0);
    lv_obj_set_style_max_height(historyList, 80, 0);
    
    btnNewChat = lv_btn_create(floatPanel);
    lv_obj_set_size(btnNewChat, LV_PCT(100), 36);
    lv_obj_set_style_bg_color(btnNewChat, lv_color_hex(0xe94560), 0);
    lv_obj_set_style_radius(btnNewChat, 8, 0);
    lv_obj_add_event_cb(btnNewChat, new_chat_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* newLabel = lv_label_create(btnNewChat);
    lv_label_set_text(newLabel, LV_SYMBOL_PLUS " New Chat");
    lv_obj_set_style_text_color(newLabel, lv_color_hex(0xffffff), 0);
    lv_obj_center(newLabel);
    
    lv_obj_t* inputContainer = lv_obj_create(floatPanel);
    lv_obj_set_size(inputContainer, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(inputContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(inputContainer, 0, 0);
    lv_obj_set_style_border_width(inputContainer, 0, 0);
    lv_obj_set_style_pad_top(inputContainer, 8, 0);
    lv_obj_set_flex_flow(inputContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(inputContainer, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    
    inputArea = lv_textarea_create(inputContainer);
    lv_obj_set_size(inputArea, LV_PCT(75), 40);
    lv_textarea_set_one_line(inputArea, true);
    lv_textarea_set_max_length(inputArea, CHAT_INPUT_MAX_LEN);
    lv_textarea_set_placeholder_text(inputArea, "Type message...");
    lv_obj_set_style_bg_color(inputArea, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_text_color(inputArea, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_border_color(inputArea, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_radius(inputArea, 8, 0);
    lv_obj_add_event_cb(inputArea, input_ready_cb, LV_EVENT_READY, this);
    
    btnSend = lv_btn_create(inputContainer);
    lv_obj_set_size(btnSend, 40, 40);
    lv_obj_set_style_bg_color(btnSend, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_radius(btnSend, 8, 0);
    lv_obj_add_event_cb(btnSend, send_btn_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* sendIcon = lv_label_create(btnSend);
    lv_label_set_text(sendIcon, LV_SYMBOL_OK);
    lv_obj_set_style_text_color(sendIcon, lv_color_hex(0xffffff), 0);
    lv_obj_center(sendIcon);
}

void ChatApp::destroyMainUI() {
    if (mainScreen) {
        lv_obj_del(mainScreen);
        mainScreen = nullptr;
    }
}

void ChatApp::destroyFloatPanel() {
}

void ChatApp::destroyUI() {
    destroyMainUI();
}

void ChatApp::showFloatPanel() {
    if (floatPanel) {
        lv_obj_clear_flag(floatPanel, LV_OBJ_FLAG_HIDDEN);
        _floatPanelOpen = true;
        updateHistoryList();
    }
}

void ChatApp::hideFloatPanel() {
    if (floatPanel) {
        lv_obj_add_flag(floatPanel, LV_OBJ_FLAG_HIDDEN);
        _floatPanelOpen = false;
    }
}

void ChatApp::toggleFloatPanel() {
    if (_floatPanelOpen) {
        hideFloatPanel();
    } else {
        showFloatPanel();
    }
}

void ChatApp::addMessage(const char* content, chat_msg_type_t type) {
    if (_activeSession < 0 || _activeSession >= _sessionCount) return;
    
    chat_session_t* session = &_sessions[_activeSession];
    if (session->msgCount >= CHAT_MAX_MESSAGES) {
        for (int i = 0; i < session->msgCount - 1; i++) {
            session->messages[i] = session->messages[i + 1];
        }
        session->msgCount--;
    }
    
    chat_message_t* msg = &session->messages[session->msgCount];
    msg->type = type;
    strncpy(msg->content, content, CHAT_MAX_MSG_LEN - 1);
    msg->content[CHAT_MAX_MSG_LEN - 1] = '\0';
    msg->timestamp = millis();
    session->msgCount++;
    session->updatedAt = millis();
    
    _needsSave = true;
    updateChatList();
}

void ChatApp::updateChatList() {
    if (!chatList || _activeSession < 0) return;
    
    lv_obj_clean(chatList);
    
    chat_session_t* session = &_sessions[_activeSession];
    
    for (int i = 0; i < session->msgCount; i++) {
        chat_message_t* msg = &session->messages[i];
        
        lv_obj_t* msgBox = lv_obj_create(chatList);
        lv_obj_set_size(msgBox, LV_PCT(100), LV_SIZE_CONTENT);
        lv_obj_set_style_pad_all(msgBox, 8, 0);
        lv_obj_set_style_radius(msgBox, 8, 0);
        lv_obj_set_style_border_width(msgBox, 0, 0);
        
        if (msg->type == CHAT_MSG_USER) {
            lv_obj_set_style_bg_color(msgBox, lv_color_hex(0x0f3460), 0);
            lv_obj_set_style_pad_left(msgBox, 12, 0);
        } else {
            lv_obj_set_style_bg_color(msgBox, lv_color_hex(0x1a1a2e), 0);
            lv_obj_set_style_pad_left(msgBox, 12, 0);
        }
        
        lv_obj_t* label = lv_label_create(msgBox);
        lv_label_set_text(label, msg->content);
        lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
        lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(label, LV_PCT(100));
    }
    
    lv_obj_scroll_to_y(chatContainer, LV_COORD_MAX, LV_ANIM_OFF);
}

void ChatApp::updateHistoryList() {
    if (!historyList) return;
    
    lv_obj_clean(historyList);
    
    for (int i = _sessionCount - 1; i >= 0; i--) {
        chat_session_t* session = &_sessions[i];
        
        lv_obj_t* item = lv_list_add_btn(historyList, LV_SYMBOL_FILE, session->name);
        lv_obj_set_style_bg_color(item, lv_color_hex(0x16213e), 0);
        lv_obj_set_style_text_color(item, lv_color_hex(0xffffff), 0);
        lv_obj_add_event_cb(item, history_select_cb, LV_EVENT_CLICKED, this);
        
        lv_obj_set_user_data(item, (void*)(intptr_t)i);
    }
}

void ChatApp::clearChatDisplay() {
    if (chatList) {
        lv_obj_clean(chatList);
    }
}

int ChatApp::createNewSession() {
    if (_sessionCount >= CHAT_MAX_SESSIONS) {
        cleanupOldSessions();
    }
    
    if (_sessionCount >= CHAT_MAX_SESSIONS) {
        return -1;
    }
    
    chat_session_t* session = &_sessions[_sessionCount];
    generateSessionId(session->id, sizeof(session->id));
    
    snprintf(session->name, CHAT_MAX_SESSION_NAME, "Chat %d", _sessionCount + 1);
    session->msgCount = 0;
    session->createdAt = millis();
    session->updatedAt = millis();
    
    _activeSession = _sessionCount;
    _sessionCount++;
    
    _needsSave = true;
    clearChatDisplay();
    updateHistoryList();
    
    return _activeSession;
}

void ChatApp::switchSession(int index) {
    if (index < 0 || index >= _sessionCount) return;
    
    _activeSession = index;
    updateChatList();
    hideFloatPanel();
}

void ChatApp::deleteSession(int index) {
    if (index < 0 || index >= _sessionCount) return;
    
    for (int i = index; i < _sessionCount - 1; i++) {
        _sessions[i] = _sessions[i + 1];
    }
    _sessionCount--;
    
    if (_activeSession == index) {
        if (_sessionCount > 0) {
            _activeSession = 0;
            updateChatList();
        } else {
            createNewSession();
        }
    } else if (_activeSession > index) {
        _activeSession--;
    }
    
    _needsSave = true;
    updateHistoryList();
}

void ChatApp::generateSessionId(char* id, size_t len) {
    snprintf(id, len, "%08lx", millis());
}

void ChatApp::checkStorage() {
    _sdAvailable = Storage.isSDReady();
    
    if (_sdAvailable) {
        if (!SD.exists(CHAT_SD_PATH)) {
            SD.mkdir(CHAT_SD_PATH);
        }
        snprintf(_storagePath, sizeof(_storagePath), "S:%s/history.json", CHAT_SD_PATH);
    } else {
        snprintf(_storagePath, sizeof(_storagePath), "F:%s", CHAT_HISTORY_FILE);
    }
}

size_t ChatApp::getAvailableSpace() {
    if (_sdAvailable) {
        return SD.totalBytes() - SD.usedBytes();
    } else {
        return SPIFFS.totalBytes() - SPIFFS.usedBytes();
    }
}

void ChatApp::cleanupOldSessions() {
    if (_sessionCount <= 1) return;
    
    int oldestIndex = 0;
    uint32_t oldestTime = _sessions[0].updatedAt;
    
    for (int i = 1; i < _sessionCount; i++) {
        if (_sessions[i].updatedAt < oldestTime) {
            oldestTime = _sessions[i].updatedAt;
            oldestIndex = i;
        }
    }
    
    deleteSession(oldestIndex);
}

bool ChatApp::saveHistory() {
    if (_sdAvailable) {
        return saveToSD();
    } else {
        return saveToSPIFFS();
    }
}

bool ChatApp::saveToSD() {
    if (!_sdAvailable) return false;
    
    String path = String(CHAT_SD_PATH) + "/history.json";
    
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        return saveToSPIFFS();
    }
    
    JsonDocument doc;
    JsonArray sessions = doc["sessions"].to<JsonArray>();
    
    for (int i = 0; i < _sessionCount; i++) {
        chat_session_t* session = &_sessions[i];
        JsonObject sessObj = sessions.add<JsonObject>();
        
        sessObj["id"] = session->id;
        sessObj["name"] = session->name;
        sessObj["created"] = session->createdAt;
        sessObj["updated"] = session->updatedAt;
        
        JsonArray msgs = sessObj["messages"].to<JsonArray>();
        for (int j = 0; j < session->msgCount; j++) {
            JsonObject msgObj = msgs.add<JsonObject>();
            msgObj["type"] = (int)session->messages[j].type;
            msgObj["content"] = session->messages[j].content;
            msgObj["time"] = session->messages[j].timestamp;
        }
    }
    
    serializeJson(doc, file);
    file.close();
    
    _needsSave = false;
    _lastSaveTime = millis();
    return true;
}

bool ChatApp::saveToSPIFFS() {
    size_t available = getAvailableSpace();
    size_t estimatedSize = _sessionCount * 1024;
    
    if (available < estimatedSize + 4096) {
        while (_sessionCount > 1 && getAvailableSpace() < estimatedSize + 4096) {
            cleanupOldSessions();
            estimatedSize = _sessionCount * 1024;
        }
    }
    
    File file = SPIFFS.open(CHAT_HISTORY_FILE, FILE_WRITE);
    if (!file) {
        return false;
    }
    
    JsonDocument doc;
    JsonArray sessions = doc["sessions"].to<JsonArray>();
    
    for (int i = 0; i < _sessionCount; i++) {
        chat_session_t* session = &_sessions[i];
        JsonObject sessObj = sessions.add<JsonObject>();
        
        sessObj["id"] = session->id;
        sessObj["name"] = session->name;
        sessObj["created"] = session->createdAt;
        sessObj["updated"] = session->updatedAt;
        
        JsonArray msgs = sessObj["messages"].to<JsonArray>();
        for (int j = 0; j < session->msgCount; j++) {
            JsonObject msgObj = msgs.add<JsonObject>();
            msgObj["type"] = (int)session->messages[j].type;
            msgObj["content"] = session->messages[j].content;
            msgObj["time"] = session->messages[j].timestamp;
        }
    }
    
    serializeJson(doc, file);
    file.close();
    
    _needsSave = false;
    _lastSaveTime = millis();
    return true;
}

bool ChatApp::loadHistory() {
    if (_sdAvailable) {
        if (loadFromSD()) return true;
    }
    return loadFromSPIFFS();
}

bool ChatApp::loadFromSD() {
    if (!_sdAvailable) return false;
    
    String path = String(CHAT_SD_PATH) + "/history.json";
    
    if (!SD.exists(path)) {
        return false;
    }
    
    File file = SD.open(path, FILE_READ);
    if (!file) {
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        return false;
    }
    
    JsonArray sessions = doc["sessions"].as<JsonArray>();
    _sessionCount = 0;
    
    for (JsonObject sessObj : sessions) {
        if (_sessionCount >= CHAT_MAX_SESSIONS) break;
        
        chat_session_t* session = &_sessions[_sessionCount];
        
        strncpy(session->id, sessObj["id"].as<const char*>(), sizeof(session->id) - 1);
        strncpy(session->name, sessObj["name"].as<const char*>(), sizeof(session->name) - 1);
        session->createdAt = sessObj["created"].as<uint32_t>();
        session->updatedAt = sessObj["updated"].as<uint32_t>();
        
        session->msgCount = 0;
        JsonArray msgs = sessObj["messages"].as<JsonArray>();
        for (JsonObject msgObj : msgs) {
            if (session->msgCount >= CHAT_MAX_MESSAGES) break;
            
            chat_message_t* msg = &session->messages[session->msgCount];
            msg->type = (chat_msg_type_t)msgObj["type"].as<int>();
            strncpy(msg->content, msgObj["content"].as<const char*>(), CHAT_MAX_MSG_LEN - 1);
            msg->timestamp = msgObj["time"].as<uint32_t>();
            session->msgCount++;
        }
        
        _sessionCount++;
    }
    
    return _sessionCount > 0;
}

bool ChatApp::loadFromSPIFFS() {
    if (!SPIFFS.exists(CHAT_HISTORY_FILE)) {
        return false;
    }
    
    File file = SPIFFS.open(CHAT_HISTORY_FILE, FILE_READ);
    if (!file) {
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        return false;
    }
    
    JsonArray sessions = doc["sessions"].as<JsonArray>();
    _sessionCount = 0;
    
    for (JsonObject sessObj : sessions) {
        if (_sessionCount >= CHAT_MAX_SESSIONS) break;
        
        chat_session_t* session = &_sessions[_sessionCount];
        
        strncpy(session->id, sessObj["id"].as<const char*>(), sizeof(session->id) - 1);
        strncpy(session->name, sessObj["name"].as<const char*>(), sizeof(session->name) - 1);
        session->createdAt = sessObj["created"].as<uint32_t>();
        session->updatedAt = sessObj["updated"].as<uint32_t>();
        
        session->msgCount = 0;
        JsonArray msgs = sessObj["messages"].as<JsonArray>();
        for (JsonObject msgObj : msgs) {
            if (session->msgCount >= CHAT_MAX_MESSAGES) break;
            
            chat_message_t* msg = &session->messages[session->msgCount];
            msg->type = (chat_msg_type_t)msgObj["type"].as<int>();
            strncpy(msg->content, msgObj["content"].as<const char*>(), CHAT_MAX_MSG_LEN - 1);
            msg->timestamp = msgObj["time"].as<uint32_t>();
            session->msgCount++;
        }
        
        _sessionCount++;
    }
    
    return _sessionCount > 0;
}

void ChatApp::sendMessage(const char* text) {
    if (!text || strlen(text) == 0) return;
    if (_waitingResponse) return;
    
    addMessage(text, CHAT_MSG_USER);
    
    if (inputArea) {
        lv_textarea_set_text(inputArea, "");
    }
    
    hideFloatPanel();
    
    _waitingResponse = true;
    
    String response = GLM.chat(text);
    
    if (response.length() > 0) {
        addMessage(response.c_str(), CHAT_MSG_AI);
    } else {
        addMessage("[No response]", CHAT_MSG_AI);
    }
    
    _waitingResponse = false;
}

void ChatApp::onUpdate() {
    if (_needsSave && millis() - _lastSaveTime > 5000) {
        saveHistory();
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

void ChatApp::float_btn_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app) {
        app->toggleFloatPanel();
    }
}

void ChatApp::send_btn_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app && app->inputArea) {
        const char* text = lv_textarea_get_text(app->inputArea);
        app->sendMessage(text);
    }
}

void ChatApp::new_chat_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app) {
        app->createNewSession();
        app->hideFloatPanel();
    }
}

void ChatApp::close_panel_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app) {
        app->hideFloatPanel();
    }
}

void ChatApp::history_select_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    lv_obj_t* item = lv_event_get_target(e);
    
    if (app && item) {
        int index = (int)(intptr_t)lv_obj_get_user_data(item);
        app->switchSession(index);
    }
}

void ChatApp::input_ready_cb(lv_event_t* e) {
    ChatApp* app = (ChatApp*)lv_event_get_user_data(e);
    if (app && app->inputArea) {
        const char* text = lv_textarea_get_text(app->inputArea);
        app->sendMessage(text);
    }
}

BaseApp* createChatApp() {
    return new ChatApp();
}
