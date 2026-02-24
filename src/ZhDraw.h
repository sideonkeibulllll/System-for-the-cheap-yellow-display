#ifndef ZH_DRAW_H
#define ZH_DRAW_H

#include <Arduino.h>
#include <LittleFS.h>
#include <lvgl.h>

class ZhDraw {
private:
    File fontFile;
    bool initialized;
    int fontSize;
    int fontPage;
    int totalChars;
    int unicodeBegin;
    char* unicodeIndex;
    
    bool loadFont(const char* path);
    int findCharIndex(uint16_t unicode);
    
public:
    ZhDraw();
    ~ZhDraw();
    
    bool begin(const char* path = "/x.font");
    void end();
    
    bool isInitialized() const { return initialized; }
    int getFontSize() const { return fontSize; }
    
    int getTextWidth(const char* text);
    int getTextHeight() const { return fontSize; }
    
    void drawText(uint16_t* buffer, int bufWidth, int bufHeight, 
                  int x, int y, const char* text, uint16_t color);
    void drawTextToCanvas(lv_obj_t* canvas, int x, int y, 
                          const char* text, uint16_t color);
    
    static ZhDraw& getInstance();
};

extern ZhDraw ZhDrawMgr;

#endif
