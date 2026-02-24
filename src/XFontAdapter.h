#ifndef XFONT_ADAPTER_H
#define XFONT_ADAPTER_H

#include <Arduino.h>
#include <SPIFFS.h>
#include <lvgl.h>

class XFontAdapter {
private:
    File fontFile;
    bool initialized;
    int fontSize;
    int binType;
    int fontPage;
    int totalChars;
    int unicodeBeginIdx;
    char* unicodeIndex;
    int unicodeIndexLen;
    
    static const char* s64;
    
    String fontPath;
    
    int findCharIndex(const char* unicodeHex);
    String getPixData(int charIndex);
    
public:
    XFontAdapter();
    ~XFontAdapter();
    
    bool begin(const char* path = "/x.font");
    void end();
    bool isInitialized() const { return initialized; }
    int getFontSize() const { return fontSize; }
    
    bool getGlyphBitmap(uint32_t unicode, uint8_t* bitmap, int* width, int* height);
    
    static XFontAdapter instance;
};

#endif
