#ifndef CHATAPP_H
#define CHATAPP_H

#include "AppManager.h"
#include "ZiranmaMapping.h"
#include "GlobalUI.h"
#include "api_config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define CHAT_INPUT_MAX_LEN      200
#define CHAT_DP_BUFFER_SIZE     24
#define CHAT_MSG_MAX_LEN        200
#define CHAT_PATH_MAX_LEN       48
#define CHAT_TEMP_FILE          "/ChatApp/.response_temp"
#define CHAT_NET_TASK_STACK     16384
#define CHAT_NET_TASK_PRIORITY  3

typedef struct ChatMessage {
    char* text;
    bool isSent;
    struct ChatMessage* next;
} ChatMessage;

class ChatApp : public BaseApp {
private:
    lv_obj_t* _blankScreen;
    lv_obj_t* _floatBtn;
    lv_obj_t* _inputArea;
    lv_obj_t* _keyboard;
    lv_obj_t* _modeBtn;
    lv_obj_t* _msgContainer;
    
    lv_obj_t* _btnOpenChat;
    lv_obj_t* _btnNewChat;
    lv_obj_t* _btnModel;
    lv_obj_t* _modelSelector;
    
    bool _inputPanelVisible;
    bool _doublePinyinMode;
    bool _sdCardAvailable;
    bool _dataFolderReady;
    bool _isWaitingResponse;
    
    char _dpBuffer[CHAT_DP_BUFFER_SIZE];
    int _dpBufferLen;
    char _preModeText[CHAT_INPUT_MAX_LEN];
    
    char _currentChatPath[CHAT_PATH_MAX_LEN];
    int _nextChatIndex;
    
    ChatMessage* _msgHead;
    ChatMessage* _msgTail;
    int _msgCount;
    
    int _selectedModelIndex;
    
    TaskHandle_t _netTaskHandle;
    char _pendingMessage[CHAT_INPUT_MAX_LEN];
    char _responseContent[CHAT_MSG_MAX_LEN];
    bool _responseReady;
    
    bool createUI() override;
    void destroyUI() override;
    bool onResume() override;
    
    void setupSidebarButtons();
    void clearSidebarButtons();
    
    bool initDataFolder();
    int getNextChatIndex();
    void appendMessageToFile(const char* text, bool isSent);
    bool loadChatFromFile(const char* path);
    void clearMessages();
    void addMessageToList(const char* text, bool isSent);
    
    void saveState() override;
    bool loadState() override;
    void checkPendingFile();
    void processPendingFile(const char* path);
    
    static void float_btn_cb(lv_event_t* e);
    static void input_focus_cb(lv_event_t* e);
    static void input_defocus_cb(lv_event_t* e);
    static void keyboard_btn_cb(lv_event_t* e);
    static void keyboard_event_cb(lv_event_t* e);
    static void sidebar_btn_cb(lv_event_t* e);
    static void model_selector_cb(lv_event_t* e);
    
    void onFloatBtnClick();
    void showInputPanel();
    void hideInputPanel();
    void toggleInputPanel();
    
    void toggleDoublePinyinMode();
    void updateModeButtonStyle();
    void processDoublePinyinInput(const char* newChars);
    void flushDpBuffer();
    void onKeyboardButtonClick(lv_obj_t* btn);
    
    lv_obj_t* createMessageBubble(lv_color_t bgColor, const char* text);
    void addMessage(const char* text, bool isSent);
    void sendMessage();
    
    void refreshMessageDisplay();
    
    void showModelSelector();
    void hideModelSelector();
    void selectModel(int index);
    void updateModelButton();
    
    bool checkNetworkConnection();
    void sendAIRequestAsync(const char* userMessage);
    static void networkTaskEntry(void* arg);
    bool performAIRequest(const char* userMessage);
    void processAIResponse();
    void parseSSELine(const char* line, char* content, int maxLen);
    
public:
    ChatApp();
    ~ChatApp();
    
    void onUpdate() override;
    app_info_t getInfo() const override;
    
    void onFileSelected(const char* path);
    void onOpenChat();
    void onNewChat();
    void onModelSelect();
};

BaseApp* createChatApp();

#endif
