#include "GLMClient.h"

GLMClient GLM;

GLMClient::GLMClient() {
    state = GLM_STATE_IDLE;
    client = nullptr;
    initialized = false;
    msgCount = 0;
    timeoutStart = 0;
    
    _model = GLM_MODEL;
    _maxTokens = GLM_MAX_TOKENS;
    _timeoutMs = GLM_TIMEOUT_MS;
    
    clearMessages();
}

GLMClient::~GLMClient() {
    end();
}

void GLMClient::setApiKey(const String& key) {
    apiKey = key;
}

void GLMClient::setApiKey(const char* key) {
    apiKey = String(key);
}

void GLMClient::setModel(const String& model) {
    _model = model;
}

void GLMClient::setMaxTokens(int tokens) {
    _maxTokens = tokens;
}

void GLMClient::setTimeout(unsigned long timeoutMs) {
    _timeoutMs = timeoutMs;
}

bool GLMClient::begin() {
    if (initialized) return true;
    
    client = new WiFiClientSecure();
    if (!client) return false;
    
    client->setInsecure();
    initialized = true;
    clearMessages();
    
    return true;
}

void GLMClient::end() {
    if (client) {
        client->stop();
        delete client;
        client = nullptr;
    }
    initialized = false;
    state = GLM_STATE_IDLE;
}

void GLMClient::addMessage(const char* role, const char* content) {
    if (msgCount >= GLM_MAX_MSG_HISTORY) {
        for (int i = 0; i < msgCount - 1; i++) {
            strcpy(messages[i].role, messages[i + 1].role);
            strcpy(messages[i].content, messages[i + 1].content);
        }
        msgCount--;
    }
    
    strncpy(messages[msgCount].role, role, 15);
    messages[msgCount].role[15] = '\0';
    strncpy(messages[msgCount].content, content, GLM_MAX_MSG_LEN - 1);
    messages[msgCount].content[GLM_MAX_MSG_LEN - 1] = '\0';
    msgCount++;
}

void GLMClient::clearMessages() {
    msgCount = 0;
    for (int i = 0; i < GLM_MAX_MSG_HISTORY; i++) {
        messages[i].role[0] = '\0';
        messages[i].content[0] = '\0';
    }
}

String GLMClient::escapeJson(const String& input) {
    String output;
    output.reserve(input.length() + 20);
    
    for (unsigned int i = 0; i < input.length(); i++) {
        char c = input.charAt(i);
        switch (c) {
            case '"': output += "\\\""; break;
            case '\\': output += "\\\\"; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:
                if (c >= 32 && c < 127) {
                    output += c;
                }
                break;
        }
    }
    return output;
}

bool GLMClient::connectToServer() {
    state = GLM_STATE_CONNECTING;
    
    if (!client || !client->connect(GLM_API_HOST, GLM_API_PORT)) {
        lastError = "Connection failed";
        state = GLM_STATE_ERROR;
        return false;
    }
    
    return true;
}

bool GLMClient::sendRequest(const String& userMessage) {
    if (!initialized && !begin()) {
        lastError = "Not initialized";
        return false;
    }
    
    if (!connectToServer()) {
        return false;
    }
    
    state = GLM_STATE_SENDING;
    
    addMessage("user", userMessage.c_str());
    
    String requestBody = "{\"model\":\"" + _model + "\",\"messages\":[";
    
    for (int i = 0; i < msgCount; i++) {
        if (i > 0) requestBody += ",";
        requestBody += "{\"role\":\"";
        requestBody += messages[i].role;
        requestBody += "\",\"content\":\"";
        requestBody += escapeJson(String(messages[i].content));
        requestBody += "\"}";
    }
    
    requestBody += "],\"max_tokens\":";
    requestBody += String(_maxTokens);
    requestBody += ",\"stream\":false}";
    
    String request = "POST " + String(GLM_API_ENDPOINT) + " HTTP/1.1\r\n";
    request += "Host: " + String(GLM_API_HOST) + "\r\n";
    request += "Authorization: Bearer " + apiKey + "\r\n";
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + String(requestBody.length()) + "\r\n";
    request += "Connection: close\r\n";
    request += "\r\n";
    request += requestBody;
    
    client->print(request);
    
    return true;
}

String GLMClient::extractContent(const String& jsonStr) {
    int contentStart = jsonStr.indexOf("\"content\":\"");
    if (contentStart == -1) return "";
    
    contentStart += 11;
    String content;
    
    for (unsigned int i = contentStart; i < jsonStr.length(); i++) {
        char c = jsonStr.charAt(i);
        if (c == '\\' && i + 1 < jsonStr.length()) {
            char next = jsonStr.charAt(i + 1);
            if (next == '"') { content += '"'; i++; }
            else if (next == 'n') { content += '\n'; i++; }
            else if (next == 'r') { content += '\r'; i++; }
            else if (next == 't') { content += '\t'; i++; }
            else if (next == '\\') { content += '\\'; i++; }
            else content += c;
        }
        else if (c == '"') {
            break;
        }
        else {
            content += c;
        }
    }
    
    return content;
}

String GLMClient::readResponse() {
    state = GLM_STATE_RECEIVING;
    String responseBuffer = "";
    timeoutStart = millis();
    
    while (millis() - timeoutStart < _timeoutMs) {
        while (client && client->available()) {
            char c = client->read();
            responseBuffer += c;
            timeoutStart = millis();
        }
        
        if (!client || !client->connected()) {
            break;
        }
        
        delay(10);
    }
    
    int bodyStart = responseBuffer.indexOf("\r\n\r\n");
    if (bodyStart == -1) {
        lastError = "Invalid response";
        state = GLM_STATE_ERROR;
        return "";
    }
    
    String body = responseBuffer.substring(bodyStart + 4);
    
    int statusStart = responseBuffer.indexOf("HTTP/1.1 ");
    if (statusStart != -1) {
        int statusCode = responseBuffer.substring(statusStart + 9, statusStart + 12).toInt();
        if (statusCode != 200) {
            lastError = "HTTP " + String(statusCode);
            state = GLM_STATE_ERROR;
            return "[Error: " + lastError + "]";
        }
    }
    
    String content = extractContent(body);
    
    if (content.length() > 0) {
        addMessage("assistant", content.c_str());
    }
    
    state = GLM_STATE_COMPLETE;
    return content;
}

String GLMClient::chat(const String& userMessage) {
    if (apiKey.length() == 0) {
        lastError = "API key not set";
        return "[Error: API key not configured]";
    }
    
    if (!sendRequest(userMessage)) {
        return "[Error: " + lastError + "]";
    }
    
    String response = readResponse();
    
    if (client) {
        client->stop();
    }
    state = GLM_STATE_IDLE;
    
    if (response.length() == 0) {
        return "[No response]";
    }
    
    return response;
}
