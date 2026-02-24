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
    lvFont.line_height = ZhFontMgr.getFontSize() + 2;
    lvFont.base_line = 0;
    lvFont.subpx = LV_FONT_SUBPX_NONE;
    lvFont.underline_position = -2;
    lvFont.underline_thickness = 1;
    lvFont.dsc = NULL;
    lvFont.fallback = NULL;
    lvFont.user_data = NULL;
    
    initialized = true;
    
    Serial.printf("[LvZhFont] Chinese font initialized, size=%d, page=%d\n", 
                  ZhFontMgr.getFontSize(), ZhFontMgr.getFontPage());
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
    
    int fontSize = ZhFontMgr.getFontSize();
    int width = (unicode_letter < 0x80) ? fontSize / 2 : fontSize;
    
    dsc_out->adv_w = width + 1;
    dsc_out->box_w = width;
    dsc_out->box_h = fontSize;
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
    
    int fontPage = ZhFontMgr.getFontPage();
    int fontSize = ZhFontMgr.getFontSize();
    int width = (unicode_letter < 0x80) ? fontSize / 2 : fontSize;
    
    uint8_t* bitmap = (uint8_t*)malloc(fontPage);
    if (!bitmap) {
        return NULL;
    }
    
    if (!ZhFontMgr.getCharBitmap((uint16_t)unicode_letter, bitmap)) {
        free(bitmap);
        return NULL;
    }
    
    const char* base64Chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#*$";
    
    memset(self->glyphBitmap, 0, sizeof(self->glyphBitmap));
    
    int outBitIdx = 0;
    for (int byteIdx = 0; byteIdx < fontPage && outBitIdx < fontSize * width; byteIdx++) {
        uint8_t val = 0;
        const char* p = strchr(base64Chars, bitmap[byteIdx]);
        if (p) val = p - base64Chars;
        
        for (int bit = 5; bit >= 0 && outBitIdx < fontSize * width; bit--) {
            bool pixel = (val >> bit) & 1;
            
            if (pixel) {
                self->glyphBitmap[outBitIdx / 8] |= (0x80 >> (outBitIdx % 8));
            }
            outBitIdx++;
        }
    }
    
    free(bitmap);
    return self->glyphBitmap;
}
