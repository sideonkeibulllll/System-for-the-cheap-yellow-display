#ifndef LV_ZH_FONT_H
#define LV_ZH_FONT_H

#include <lvgl.h>
#include "XFontAdapter.h"

class LvZhFont {
private:
    static lv_font_t fontDescriptor;
    static uint8_t glyphBitmap[1024];
    static lv_font_glyph_dsc_t glyphDsc;
    static bool initialized;
    
    static unsigned int statGetGlyphDsc;
    static unsigned int statGetGlyphBitmapLv;
    
    static bool getGlyphDsc(const lv_font_t* font, lv_font_glyph_dsc_t* dsc, uint32_t unicode, uint32_t unicode_next);
    static const uint8_t* getGlyphBitmap(const lv_font_t* font, uint32_t unicode);
    
public:
    static bool begin();
    static lv_font_t* getFont() { return &fontDescriptor; }
    static bool isInitialized() { return initialized; }
    static unsigned int getStatGetGlyphDsc() { return statGetGlyphDsc; }
    static unsigned int getStatGetGlyphBitmapLv() { return statGetGlyphBitmapLv; }
    static void resetStats() { statGetGlyphDsc = 0; statGetGlyphBitmapLv = 0; }
};

extern LvZhFont LvZhFontMgr;

#endif
