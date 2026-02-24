#include "ZhDraw.h"
#include <lvgl.h>

ZhDraw ZhDrawMgr;

static const char* base64Chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#*$";

ZhDraw::ZhDraw() {
    initialized = false;
    fontSize = 16;
    fontPage = 48;
    totalChars = 0;
    unicodeBegin = 0;
    unicodeIndex = nullptr;
}

ZhDraw::~ZhDraw() {
    end();
}

ZhDraw& ZhDraw::getInstance() {
    return ZhDrawMgr;
}

bool ZhDraw::begin(const char* path) {
    if (initialized) return true;
    
    if (!LittleFS.begin(true)) {
        Serial.println("[ZhDraw] LittleFS mount failed");
        return false;
    }
    
    if (!LittleFS.exists(path)) {
        Serial.printf("[ZhDraw] Font not found: %s\n", path);
        return false;
    }
    
    fontFile = LittleFS.open(path, "r");
    if (!fontFile) {
        Serial.println("[ZhDraw] Failed to open font");
        return false;
    }
    
    uint8_t header[10];
    fontFile.readBytes((char*)header, 10);
    
    char totalStr[7] = {0};
    memcpy(totalStr, header, 6);
    totalChars = strtol(totalStr, NULL, 16);
    
    fontSize = (header[6] - '0') * 10 + (header[7] - '0');
    if (fontSize < 12 || fontSize > 24) fontSize = 16;
    
    int binType = (header[8] - '0') * 10 + (header[9] - '0');
    int totalPixels = fontSize * fontSize;
    int bitsPerChar = 6;
    if (binType == 32) bitsPerChar = 5;
    fontPage = (totalPixels + bitsPerChar - 1) / bitsPerChar;
    
    unicodeBegin = 10 + totalChars * 5;
    
    initialized = true;
    Serial.printf("[ZhDraw] Ready: size=%d, chars=%d (no index loaded)\n", fontSize, totalChars);
    return true;
}

void ZhDraw::end() {
    if (fontFile) {
        fontFile.close();
    }
    if (unicodeIndex) {
        free(unicodeIndex);
        unicodeIndex = nullptr;
    }
    initialized = false;
}

int ZhDraw::findCharIndex(uint16_t unicode) {
    char search[6];
    snprintf(search, sizeof(search), "u%04X", unicode);
    
    fontFile.seek(10);
    
    int bufSize = 256;
    char buf[256];
    int remaining = totalChars * 5;
    int offset = 0;
    
    while (remaining > 0) {
        int toRead = (remaining > bufSize) ? bufSize : remaining;
        fontFile.readBytes(buf, toRead);
        
        for (int i = 0; i <= toRead - 5; i += 5) {
            if (buf[i] == 'u' && strncmp(&buf[i], search, 5) == 0) {
                return (offset + i) / 5;
            }
        }
        
        offset += toRead;
        remaining -= toRead;
    }
    
    return -1;
}

int ZhDraw::getTextWidth(const char* text) {
    if (!text) return 0;
    
    int width = 0;
    const uint8_t* p = (const uint8_t*)text;
    
    while (*p) {
        if (*p < 0x80) {
            width += fontSize / 2 + 1;
            p++;
        } else {
            if ((p[0] & 0xE0) == 0xC0) p += 2;
            else if ((p[0] & 0xF0) == 0xE0) p += 3;
            else p++;
            width += fontSize + 1;
        }
    }
    
    return width;
}

void ZhDraw::drawText(uint16_t* buffer, int bufWidth, int bufHeight, 
                       int startX, int startY, const char* text, uint16_t color) {
    if (!initialized || !text || !buffer) return;
    
    int px = startX;
    int py = startY;
    const uint8_t* p = (const uint8_t*)text;
    
    uint8_t* charData = (uint8_t*)malloc(fontPage);
    if (!charData) return;
    
    while (*p) {
        uint16_t unicode = 0;
        int charWidth = fontSize;
        
        if (*p < 0x80) {
            unicode = *p++;
            charWidth = fontSize / 2;
        } else {
            if ((p[0] & 0xE0) == 0xC0 && p[1]) {
                unicode = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
                p += 2;
            } else if ((p[0] & 0xF0) == 0xE0 && p[1] && p[2]) {
                unicode = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
                p += 3;
            } else {
                p++;
                continue;
            }
        }
        
        int charIdx = findCharIndex(unicode);
        if (charIdx >= 0) {
            int dataOffset = unicodeBegin + charIdx * fontPage;
            fontFile.seek(dataOffset);
            fontFile.readBytes((char*)charData, fontPage);
            
            int pixelIdx = 0;
            for (int byteIdx = 0; byteIdx < fontPage && pixelIdx < fontSize * fontSize; byteIdx++) {
                const char* found = strchr(base64Chars, charData[byteIdx]);
                int val = found ? (found - base64Chars) : 0;
                
                for (int bit = 5; bit >= 0 && pixelIdx < fontSize * fontSize; bit--) {
                    int row = pixelIdx / fontSize;
                    int col = pixelIdx % fontSize;
                    
                    if (col < charWidth) {
                        int bufX = px + col;
                        int bufY = py + row;
                        
                        if (bufX >= 0 && bufX < bufWidth && bufY >= 0 && bufY < bufHeight) {
                            if ((val >> bit) & 1) {
                                buffer[bufY * bufWidth + bufX] = color;
                            }
                        }
                    }
                    pixelIdx++;
                }
            }
        }
        
        px += charWidth + 1;
    }
    
    free(charData);
}

void ZhDraw::drawTextToCanvas(lv_obj_t* canvas, int x, int y, 
                               const char* text, uint16_t color) {
    if (!canvas || !text) return;
    
    lv_img_dsc_t* dsc = (lv_img_dsc_t*)lv_canvas_get_img(canvas);
    if (!dsc || !dsc->data) return;
    
    uint16_t* buffer = (uint16_t*)dsc->data;
    int width = dsc->header.w;
    int height = dsc->header.h;
    
    drawText(buffer, width, height, x, y, text, color);
}
