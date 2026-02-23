#include "LvZhFont.h"

LvZhFont LvZhFontMgr;

LvZhFont::LvZhFont() {
    initialized = false;
}

LvZhFont::~LvZhFont() {
}

LvZhFont& LvZhFont::getInstance() {
    return LvZhFontMgr;
}

bool LvZhFont::begin() {
    if (initialized) return true;
    
    if (!ZhFontMgr.begin("/x.font")) {
        Serial.println("[LvZhFont] Failed to init ZhFont");
        return false;
    }
    
    lvFont.get_glyph_dsc = getGlyphCb;
    lvFont.get_glyph_bitmap = getGlyphBitmapCb;
    lvFont.line_height = 18;
    lvFont.base_line = 0;
    lvFont.subpx = LV_FONT_SUBPX_NONE;
    lvFont.underline_position = -2;
    lvFont.underline_thickness = 1;
    lvFont.dsc = NULL;
    lvFont.fallback = NULL;
    lvFont.user_data = NULL;
    
    initialized = true;
    
    Serial.println("[LvZhFont] Chinese font initialized");
    return true;
}

bool LvZhFont::getGlyphCb(const lv_font_t* font, lv_font_glyph_dsc_t* dsc_out, 
                          uint32_t unicode_letter, uint32_t unicode_letter_next) {
    (void)font;
    (void)unicode_letter_next;
    
    if (unicode_letter < 0x20) {
        dsc_out->adv_w = 0;
        dsc_out->box_w = 0;
        dsc_out->box_h = 0;
        dsc_out->ofs_x = 0;
        dsc_out->ofs_y = 0;
        dsc_out->bpp = 0;
        dsc_out->is_placeholder = false;
        return true;
    }
    
    int width = (unicode_letter < 0x80) ? 8 : 16;
    
    dsc_out->adv_w = width + 1;
    dsc_out->box_w = width;
    dsc_out->box_h = 16;
    dsc_out->ofs_x = 0;
    dsc_out->ofs_y = 0;
    dsc_out->bpp = 1;
    dsc_out->is_placeholder = false;
    
    return true;
}

const uint8_t* LvZhFont::getGlyphBitmapCb(const lv_font_t* font, uint32_t unicode_letter) {
    LvZhFont* self = &LvZhFontMgr;
    
    if (unicode_letter < 0x20) {
        return NULL;
    }
    
    uint8_t bitmap[64];
    if (!ZhFontMgr.getCharBitmap((uint16_t)unicode_letter, bitmap)) {
        return NULL;
    }
    
    int fontPage = ZhFontMgr.getFontPage();
    int width = (unicode_letter < 0x80) ? 8 : 16;
    
    const char* base64Chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#*$";
    
    memset(self->glyphBitmap, 0, sizeof(self->glyphBitmap));
    
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < width; col++) {
            int bitIdx = row * 16 + col;
            int byteIdx = bitIdx / 6;
            int bitOffset = 5 - (bitIdx % 6);
            
            if (byteIdx >= fontPage) continue;
            
            uint8_t val = 0;
            const char* p = strchr(base64Chars, bitmap[byteIdx]);
            if (p) val = p - base64Chars;
            
            bool pixel = (val >> bitOffset) & 1;
            
            if (pixel) {
                int outBitIdx = row * width + col;
                self->glyphBitmap[outBitIdx / 8] |= (0x80 >> (outBitIdx % 8));
            }
        }
    }
    
    return self->glyphBitmap;
}
