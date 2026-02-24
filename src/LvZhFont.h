#ifndef LV_ZH_FONT_H
#define LV_ZH_FONT_H

#include <lvgl.h>
#include "ZhFont.h"

class LvZhFont {
private:
    lv_font_t lvFont;
    uint8_t glyphBitmap[128];
    bool initialized;
    
    static bool getGlyphCb(const lv_font_t* font, lv_font_glyph_dsc_t* dsc_out, 
                           uint32_t unicode_letter, uint32_t unicode_letter_next);
    static const uint8_t* getGlyphBitmapCb(const lv_font_t* font, uint32_t unicode_letter);
    
public:
    LvZhFont();
    ~LvZhFont();
    
    bool begin();
    lv_font_t* getFont() { return &lvFont; }
    uint8_t* getGlyphBuffer() { return glyphBitmap; }
    bool isInitialized() const { return initialized; }
    
    static LvZhFont& getInstance();
};

extern LvZhFont LvZhFontMgr;

#endif
