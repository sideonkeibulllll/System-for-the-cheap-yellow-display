#include "LvZhFont.h"

lv_font_t LvZhFont::fontDescriptor;
uint8_t LvZhFont::glyphBitmap[256];
lv_font_glyph_dsc_t LvZhFont::glyphDsc;
bool LvZhFont::initialized = false;

LvZhFont LvZhFontMgr;

bool LvZhFont::begin() {
    if (initialized) return true;
    
    if (!XFontAdapter::instance.begin()) {
        Serial.println("[LvZhFont] XFontAdapter init failed");
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
    Serial.printf("[LvZhFont] Initialized with fontSize=%d\n", XFontAdapter::instance.getFontSize());
    
    return true;
}

bool LvZhFont::getGlyphDsc(const lv_font_t* font, lv_font_glyph_dsc_t* dsc, uint32_t unicode, uint32_t unicode_next) {
    if (!XFontAdapter::instance.isInitialized()) {
        Serial.println("[LvZhFont] getGlyphDsc: not initialized");
        return false;
    }
    
    static uint32_t lastUnicode = 0;
    if (unicode != lastUnicode) {
        Serial.printf("[LvZhFont] getGlyphDsc: unicode=0x%04X (%c)\n", unicode, unicode < 128 ? unicode : '?');
        lastUnicode = unicode;
    }
    
    int width, height;
    if (!XFontAdapter::instance.getGlyphBitmap(unicode, glyphBitmap, &width, &height)) {
        Serial.printf("[LvZhFont] getGlyphDsc: getGlyphBitmap failed for 0x%04X\n", unicode);
        return false;
    }
    
    dsc->adv_w = width;
    dsc->box_w = width;
    dsc->box_h = height;
    dsc->ofs_x = 0;
    dsc->ofs_y = 0;
    dsc->bpp = 1;
    dsc->is_placeholder = 0;
    
    return true;
}

const uint8_t* LvZhFont::getGlyphBitmap(const lv_font_t* font, uint32_t unicode) {
    int width, height;
    XFontAdapter::instance.getGlyphBitmap(unicode, glyphBitmap, &width, &height);
    
    static uint8_t packedBitmap[288];
    memset(packedBitmap, 0, sizeof(packedBitmap));
    
    int totalBits = width * height;
    for (int i = 0; i < totalBits; i++) {
        if (glyphBitmap[i]) {
            packedBitmap[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    
    return packedBitmap;
}
