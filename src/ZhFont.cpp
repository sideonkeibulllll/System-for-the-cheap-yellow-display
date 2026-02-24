#include "ZhFont.h"

ZhFont ZhFontMgr;

ZhFont::ZhFont() {
    initialized = false;
    fontSize = ZH_FONT_SIZE;
    fontPage = ZH_FONT_PAGE;
    totalChars = 0;
    unicodeBegin = 0;
    unicodeIndex = nullptr;
    unicodeIndexSize = 0;
    cacheIndex = 0;
    
    for (int i = 0; i < ZH_MAX_CACHE; i++) {
        cacheValid[i] = false;
    }
}

ZhFont::~ZhFont() {
    end();
}

ZhFont& ZhFont::getInstance() {
    return ZhFontMgr;
}

bool ZhFont::begin(const char* fontPath) {
    if (initialized) return true;
    
    if (!SPIFFS.begin(true)) {
        Serial.println("[ZhFont] SPIFFS mount failed");
        return false;
    }
    
    if (!SPIFFS.exists(fontPath)) {
        Serial.printf("[ZhFont] Font file not found: %s\n", fontPath);
        return false;
    }
    
    fontFile = SPIFFS.open(fontPath, "r");
    if (!fontFile) {
        Serial.println("[ZhFont] Failed to open font file");
        return false;
    }
    
    if (!loadFontHeader()) {
        fontFile.close();
        return false;
    }
    
    if (!loadUnicodeIndex()) {
        fontFile.close();
        return false;
    }
    
    initialized = true;
    Serial.printf("[ZhFont] Initialized: size=%d, chars=%d, indexSize=%d\n", 
                  fontSize, totalChars, unicodeIndexSize);
    return true;
}

void ZhFont::end() {
    if (fontFile) {
        fontFile.close();
    }
    if (unicodeIndex) {
        free(unicodeIndex);
        unicodeIndex = nullptr;
    }
    initialized = false;
}

bool ZhFont::loadFontHeader() {
    fontFile.seek(0);
    
    uint8_t buf[10];
    fontFile.readBytes((char*)buf, 10);
    
    char totalStr[7] = {0};
    memcpy(totalStr, buf, 6);
    totalChars = strtol(totalStr, NULL, 16);
    
    fontSize = (buf[6] - '0') * 10 + (buf[7] - '0');
    if (fontSize < 12 || fontSize > 24) fontSize = ZH_FONT_SIZE;
    
    fontPage = (buf[8] - '0') * 10 + (buf[9] - '0');
    if (fontPage < 32 || fontPage > 64) fontPage = ZH_FONT_PAGE;
    
    unicodeBegin = 10 + totalChars * 5;
    
    return true;
}

bool ZhFont::loadUnicodeIndex() {
    unicodeIndexSize = totalChars * 5;
    unicodeIndex = (char*)malloc(unicodeIndexSize + 1);
    
    if (!unicodeIndex) {
        Serial.println("[ZhFont] Failed to allocate unicode index");
        return false;
    }
    
    fontFile.seek(10);
    int bytesRead = fontFile.readBytes(unicodeIndex, unicodeIndexSize);
    unicodeIndex[unicodeIndexSize] = '\0';
    
    if (bytesRead != unicodeIndexSize) {
        Serial.printf("[ZhFont] Read error: expected %d, got %d\n", unicodeIndexSize, bytesRead);
        free(unicodeIndex);
        unicodeIndex = nullptr;
        return false;
    }
    
    Serial.printf("[ZhFont] Loaded %d chars index (%d bytes)\n", totalChars, unicodeIndexSize);
    return true;
}

int ZhFont::findCharIndex(uint16_t unicode) {
    if (!unicodeIndex) return -1;
    
    char search[6];
    snprintf(search, sizeof(search), "u%04X", unicode);
    
    char* found = strstr(unicodeIndex, search);
    if (!found) return -1;
    
    return (found - unicodeIndex) / 5;
}

bool ZhFont::readCharData(int index, uint8_t* data) {
    int offset = unicodeBegin + index * fontPage;
    fontFile.seek(offset);
    fontFile.readBytes((char*)data, fontPage);
    return true;
}

int ZhFont::findInCache(uint16_t unicode) {
    for (int i = 0; i < ZH_MAX_CACHE; i++) {
        if (cacheValid[i] && cacheUnicode[i] == unicode) {
            return i;
        }
    }
    return -1;
}

void ZhFont::addToCache(uint16_t unicode, const uint8_t* data) {
    cacheUnicode[cacheIndex] = unicode;
    cacheValid[cacheIndex] = true;
    memcpy(cacheData[cacheIndex], data, fontPage);
    
    cacheIndex = (cacheIndex + 1) % ZH_MAX_CACHE;
}

bool ZhFont::getCharBitmap(uint16_t unicode, uint8_t* bitmap) {
    if (!initialized) return false;
    
    int cacheIdx = findInCache(unicode);
    if (cacheIdx >= 0) {
        memcpy(bitmap, cacheData[cacheIdx], fontPage);
        return true;
    }
    
    int charIdx = findCharIndex(unicode);
    if (charIdx < 0) return false;
    
    if (!readCharData(charIdx, bitmap)) return false;
    
    addToCache(unicode, bitmap);
    return true;
}
