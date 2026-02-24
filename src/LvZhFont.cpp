#include "LvZhFont.h"

lv_font_t LvZhFont::fontDescriptor;
uint8_t LvZhFont::glyphBitmap[1024];
lv_font_glyph_dsc_t LvZhFont::glyphDsc;
bool LvZhFont::initialized = false;
unsigned int LvZhFont::statGetGlyphDsc = 0;
unsigned int LvZhFont::statGetGlyphBitmapLv = 0;

LvZhFont LvZhFontMgr;

bool LvZhFont::begin() {
    if (initialized) return true;
    
    if (!XFontAdapter::instance.begin()) {
        return false;
    }
    
    memset(&fontDescriptor, 0, sizeof(lv_font_t));
    fontDescriptor.get_glyph_dsc = getGlyphDsc;
    fontDescriptor.get_glyph_bitmap = getGlyphBitmap;
    fontDescriptor.line_height = XFontAdapter::instance.getFontSize();
    fontDescriptor.base_line = 0;
    fontDescriptor.dsc = NULL;
    fontDescriptor.fallback = NULL;
    fontDescriptor.subpx = LV_FONT_SUBPX_NONE;
    
    initialized = true;
    
    return true;
}

bool LvZhFont::getGlyphDsc(const lv_font_t* font, lv_font_glyph_dsc_t* dsc, uint32_t unicode, uint32_t unicode_next) {
    statGetGlyphDsc++;
    
    if (!XFontAdapter::instance.isInitialized()) {
        return false;
    }
    
    int fontSize = XFontAdapter::instance.getFontSize();
    bool isAscii = (unicode <= 127);
    
    dsc->adv_w = isAscii ? fontSize / 2 : fontSize;
    dsc->box_w = isAscii ? fontSize / 2 : fontSize;
    dsc->box_h = fontSize;
    dsc->ofs_x = 0;
    dsc->ofs_y = 0;
    dsc->bpp = 1;
    dsc->is_placeholder = 0;
    
    return true;
}

const uint8_t* LvZhFont::getGlyphBitmap(const lv_font_t* font, uint32_t unicode) {
    statGetGlyphBitmapLv++;
    
    int width, height;
    XFontAdapter::instance.getGlyphBitmap(unicode, glyphBitmap, &width, &height);
    
    static uint8_t packedBitmap[1024];
    memset(packedBitmap, 0, sizeof(packedBitmap));
    
    int totalBits = width * height;
    for (int i = 0; i < totalBits; i++) {
        if (glyphBitmap[i]) {
            packedBitmap[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    
    XFontAdapter::instance.addPackBitmapCount(totalBits);
    
    return packedBitmap;
}
