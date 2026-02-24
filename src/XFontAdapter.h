#ifndef XFONT_ADAPTER_H
#define XFONT_ADAPTER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <lvgl.h>
#include <pgmspace.h>

#define CACHE_SIZE 16

class XFontAdapter {
private:
    bool initialized;
    int fontSize;
    int binType;
    int fontPage;
    int totalChars;
    int unicodeBeginIdx;
    
    static const char* s64;
    
    String fontPath;
    
    File fontFile;
    bool fileOpen;
    unsigned long lastFileAccess;
    
    static uint8_t pixBuf[128];
    
    static uint32_t cacheUnicode[CACHE_SIZE];
    static uint8_t cacheBitmap[CACHE_SIZE][72];
    static int cacheIndex;
    
    static unsigned long statLastPrint;
    static unsigned int statFindCharIndex;
    static unsigned int statReadPixData;
    static unsigned int statGetGlyphBitmap;
    static unsigned int statDecodePixel;
    static unsigned int statCacheHit;
    
    int findCharIndex(uint32_t unicode);
    bool readPixData(int charIndex);
    void checkFileOpen();
    void checkFileClose();
    void printStats();
    int findInCache(uint32_t unicode);
    void addToCache(uint32_t unicode, const uint8_t* bitmap);
    
public:
    XFontAdapter();
    ~XFontAdapter();
    
    bool begin(const char* path = "/x.font");
    void end();
    void update();
    bool isInitialized() const { return initialized; }
    int getFontSize() const { return fontSize; }
    
    const uint8_t* getGlyphBitmapPacked(uint32_t unicode, int* width, int* height);
    
    static XFontAdapter instance;
};

#endif
