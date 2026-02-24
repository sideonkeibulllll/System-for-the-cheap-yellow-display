#ifndef XFONT_ADAPTER_H
#define XFONT_ADAPTER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <lvgl.h>
#include <pgmspace.h>

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
    static uint8_t tempBitmap[576];
    
    static unsigned long statLastPrint;
    static unsigned int statFindCharIndex;
    static unsigned int statReadPixData;
    static unsigned int statGetGlyphBitmap;
    static unsigned int statDecodePixel;
    static unsigned int statPackBitmap;
    
    int findCharIndex(uint32_t unicode);
    bool readPixData(int charIndex);
    void checkFileOpen();
    void checkFileClose();
    void printStats();
    
public:
    XFontAdapter();
    ~XFontAdapter();
    
    bool begin(const char* path = "/x.font");
    void end();
    void update();
    bool isInitialized() const { return initialized; }
    int getFontSize() const { return fontSize; }
    
    bool getGlyphBitmap(uint32_t unicode, uint8_t* bitmap, int* width, int* height);
    void addPackBitmapCount(int count) { statPackBitmap += count; }
    
    static XFontAdapter instance;
};

#endif
