#ifndef GLMCLIENT_H
#define GLMCLIENT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>

#define GLM_API_HOST        "open.bigmodel.cn"
#define GLM_API_PORT        443
#define GLM_API_ENDPOINT    "/api/paas/v4/chat/completions"
#define GLM_MODEL           "glm-4.7"
#define GLM_MAX_TOKENS      512
#define GLM_TIMEOUT_MS      30000
#define GLM_MAX_MSG_HISTORY 10
#define GLM_MAX_MSG_LEN     256

typedef enum {
    GLM_STATE_IDLE = 0,
    GLM_STATE_CONNECTING,
    GLM_STATE_SENDING,
    GLM_STATE_RECEIVING,
    GLM_STATE_COMPLETE,
    GLM_STATE_ERROR
} glm_state_t;

class GLMClient {
private:
    WiFiClientSecure* client;
    String apiKey;
    glm_state_t state;
    
    String lastError;
    unsigned long timeoutStart;
    bool initialized;
    
    struct {
        char role[16];
        char content[GLM_MAX_MSG_LEN];
    } messages[GLM_MAX_MSG_HISTORY];
    int msgCount;
    
    String _model;
    int _maxTokens;
    unsigned long _timeoutMs;
    
    bool connectToServer();
    bool sendRequest(const String& userMessage);
    String readResponse();
    String extractContent(const String& jsonStr);
    String escapeJson(const String& input);
    
public:
    GLMClient();
    ~GLMClient();
    
    void setApiKey(const String& key);
    void setApiKey(const char* key);
    
    bool begin();
    void end();
    
    void addMessage(const char* role, const char* content);
    void clearMessages();
    
    String chat(const String& userMessage);
    
    glm_state_t getState() const { return state; }
    String getLastError() const { return lastError; }
    
    void setModel(const String& model);
    void setMaxTokens(int tokens);
    void setTimeout(unsigned long timeoutMs);
};

extern GLMClient GLM;

#endif
