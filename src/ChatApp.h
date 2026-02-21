#ifndef CHATAPP_H
#define CHATAPP_H

#include "AppManager.h"
#include "GLMClient.h"

#define CHAT_MAX_MESSAGES       50
#define CHAT_MAX_MSG_LEN        512
#define CHAT_MAX_SESSIONS       10
#define CHAT_MAX_SESSION_NAME   32
#define CHAT_INPUT_MAX_LEN      256
#define CHAT_HISTORY_FILE       "/chat_history.json"
#define CHAT_SD_PATH            "/chat"

#define GLM_API_KEY             "adda42a211694ce9a6ad7a1ced961485.1kCZvMxkHAA6EKkn"

typedef enum {
    CHAT_MSG_USER = 0,
    CHAT_MSG_AI = 1
} chat_msg_type_t;

typedef struct {
    chat_msg_type_t type;
    char content[CHAT_MAX_MSG_LEN];
    uint32_t timestamp;
} chat_message_t;

typedef struct {
    char id[16];
    char name[CHAT_MAX_SESSION_NAME];
    chat_message_t messages[CHAT_MAX_MESSAGES];
    int msgCount;
    uint32_t createdAt;
    uint32_t updatedAt;
} chat_session_t;

class ChatApp : public BaseApp {
private:
    lv_obj_t* mainScreen;
    lv_obj_t* chatContainer;
    lv_obj_t* chatList;
    
    lv_obj_t* floatBtn;
    lv_obj_t* floatPanel;
    lv_obj_t* historyList;
    lv_obj_t* btnNewChat;
    lv_obj_t* inputArea;
    lv_obj_t* btnSend;
    lv_obj_t* btnClose;
    
    chat_session_t _sessions[CHAT_MAX_SESSIONS];
    int _sessionCount;
    int _activeSession;
    
    bool _floatPanelOpen;
    bool _sdAvailable;
    char _storagePath[64];
    
    uint32_t _lastSaveTime;
    bool _needsSave;
    bool _waitingResponse;
    
    static void float_btn_cb(lv_event_t* e);
    static void send_btn_cb(lv_event_t* e);
    static void new_chat_cb(lv_event_t* e);
    static void close_panel_cb(lv_event_t* e);
    static void history_select_cb(lv_event_t* e);
    static void input_ready_cb(lv_event_t* e);
    
    void createMainUI();
    void createFloatPanel();
    void destroyMainUI();
    void destroyFloatPanel();
    
    void showFloatPanel();
    void hideFloatPanel();
    void toggleFloatPanel();
    
    void addMessage(const char* content, chat_msg_type_t type);
    void updateChatList();
    void updateHistoryList();
    void clearChatDisplay();
    
    int createNewSession();
    void switchSession(int index);
    void deleteSession(int index);
    void generateSessionId(char* id, size_t len);
    
    bool saveHistory();
    bool loadHistory();
    bool saveToSD();
    bool saveToSPIFFS();
    bool loadFromSD();
    bool loadFromSPIFFS();
    
    void checkStorage();
    size_t getAvailableSpace();
    void cleanupOldSessions();
    
    void sendMessage(const char* text);
    
protected:
    virtual bool createUI() override;
    virtual void destroyUI() override;
    
public:
    ChatApp();
    virtual ~ChatApp();
    
    virtual void onUpdate() override;
    virtual app_info_t getInfo() const override;
};

BaseApp* createChatApp();

#endif
