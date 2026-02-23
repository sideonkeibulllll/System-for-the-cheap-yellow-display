#include "ZhFont.h"

ZhFont ZhFontMgr;

ZhFont::ZhFont() {
    initialized = false;
    fontSize = ZH_FONT_SIZE;
    fontPage = ZH_FONT_PAGE;
    totalChars = 0;
    unicodeBegin = 0;
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
    
    initialized = true;
    Serial.printf("[ZhFont] Initialized: size=%d, chars=%d\n", fontSize, totalChars);
    return true;
}

void ZhFont::end() {
    if (fontFile) {
        fontFile.close();
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

int ZhFont::findCharIndex(uint16_t unicode) {
    char search[6];
    snprintf(search, sizeof(search), "u%04X", unicode);
    
    fontFile.seek(10);
    
    int bufSize = 256;
    char buf[256];
    int remaining = totalChars * 5;
    int offset = 0;
    
    while (remaining > 0) {
        int toRead = (remaining > bufSize) ? bufSize : remaining;
        fontFile.readBytes(buf, toRead);
        
        for (int i = 0; i <= toRead - 5; i += 5) {
            if (buf[i] == 'u' && 
                strncmp(&buf[i], search, 5) == 0) {
                return (offset + i) / 5;
            }
        }
        
        offset += toRead;
        remaining -= toRead;
    }
    
    return -1;
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
