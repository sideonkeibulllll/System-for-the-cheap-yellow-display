#ifndef ZHFONT_H
#define ZHFONT_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#define ZH_FONT_SIZE        16
#define ZH_FONT_PAGE        48
#define ZH_MAX_CACHE        8

class ZhFont {
private:
    File fontFile;
    bool initialized;
    int fontSize;
    int fontPage;
    int totalChars;
    int unicodeBegin;
    
    uint16_t cacheUnicode[ZH_MAX_CACHE];
    uint8_t cacheData[ZH_MAX_CACHE][ZH_FONT_PAGE];
    bool cacheValid[ZH_MAX_CACHE];
    int cacheIndex;
    
    bool loadFontHeader();
    int findCharIndex(uint16_t unicode);
    bool readCharData(int index, uint8_t* data);
    int findInCache(uint16_t unicode);
    void addToCache(uint16_t unicode, const uint8_t* data);
    
public:
    ZhFont();
    ~ZhFont();
    
    bool begin(const char* fontPath = "/x.font");
    void end();
    
    bool isInitialized() const { return initialized; }
    int getFontSize() const { return fontSize; }
    int getFontPage() const { return fontPage; }
    
    bool getCharBitmap(uint16_t unicode, uint8_t* bitmap);
    
    static ZhFont& getInstance();
};

extern ZhFont ZhFontMgr;

#endif
