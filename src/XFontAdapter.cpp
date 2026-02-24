#include "XFontAdapter.h"
#include "font_index_map.h"
#include "LvZhFont.h"

const char* XFontAdapter::s64 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#*$";
uint8_t XFontAdapter::pixBuf[128];
uint8_t XFontAdapter::tempBitmap[576];

unsigned long XFontAdapter::statLastPrint = 0;
unsigned int XFontAdapter::statFindCharIndex = 0;
unsigned int XFontAdapter::statReadPixData = 0;
unsigned int XFontAdapter::statGetGlyphBitmap = 0;
unsigned int XFontAdapter::statDecodePixel = 0;
unsigned int XFontAdapter::statPackBitmap = 0;

XFontAdapter XFontAdapter::instance;

XFontAdapter::XFontAdapter() {
    initialized = false;
    fontSize = 0;
    binType = 0;
    fontPage = 0;
    totalChars = 0;
    unicodeBeginIdx = 0;
    fontPath = "/x.font";
    fileOpen = false;
    lastFileAccess = 0;
}

XFontAdapter::~XFontAdapter() {
    end();
}

void XFontAdapter::end() {
    if (fileOpen && fontFile) {
        fontFile.close();
        fileOpen = false;
    }
    initialized = false;
}

bool XFontAdapter::begin(const char* path) {
    if (initialized) return true;
    
    fontPath = path;
    
    if (!SPIFFS.begin()) {
        return false;
    }
    
    if (!SPIFFS.exists(fontPath)) {
        return false;
    }
    
    File f = SPIFFS.open(fontPath, "r");
    if (!f) {
        return false;
    }
    
    uint8_t bufTotalStr[6];
    uint8_t bufFontSize[2];
    uint8_t bufBinType[2];
    
    f.read(bufTotalStr, 6);
    f.read(bufFontSize, 2);
    f.read(bufBinType, 2);
    
    String s1;
    for (int i = 0; i < 6; i++) {
        s1 += (char)bufTotalStr[i];
    }
    
    fontSize = (bufFontSize[0] - '0') * 10 + (bufFontSize[1] - '0');
    if (fontSize < 10 || fontSize > 50) {
        fontSize = bufFontSize[0] - '0';
    }
    
    binType = (bufBinType[0] - '0') * 10 + (bufBinType[1] - '0');
    if (binType < 10 || binType > 100) {
        binType = bufBinType[0] - '0';
    }
    
    totalChars = strtol(s1.c_str(), NULL, 16);
    
    int total = fontSize * fontSize;
    int hexCount = 12;
    int hexAmount = (total + hexCount - 1) / hexCount;
    fontPage = hexAmount * 2;
    
    unicodeBeginIdx = 10 + totalChars * 5;
    
    f.close();
    
    initialized = true;
    
    return true;
}

void XFontAdapter::checkFileOpen() {
    if (!fileOpen) {
        fontFile = SPIFFS.open(fontPath, "r");
        fileOpen = true;
    }
    lastFileAccess = millis();
}

void XFontAdapter::checkFileClose() {
    if (fileOpen && millis() - lastFileAccess > 10000) {
        fontFile.close();
        fileOpen = false;
    }
}

void XFontAdapter::update() {
    checkFileClose();
    printStats();
}

void XFontAdapter::printStats() {
    if (millis() - statLastPrint >= 10000) {
        Serial.println("\n========== 字体性能统计 (每10秒) ==========");
        Serial.println("--- LVGL层 ---");
        Serial.printf("getGlyphDsc:    %u 次\n", LvZhFont::getStatGetGlyphDsc());
        Serial.printf("getGlyphBitmap: %u 次\n", LvZhFont::getStatGetGlyphBitmapLv());
        Serial.println("--- XFont层 ---");
        Serial.printf("findCharIndex:  %u 次\n", statFindCharIndex);
        Serial.printf("readPixData:    %u 次\n", statReadPixData);
        Serial.printf("getGlyphBitmap: %u 次\n", statGetGlyphBitmap);
        Serial.println("--- 像素处理 ---");
        Serial.printf("decodePixel:    %u 次\n", statDecodePixel);
        Serial.printf("packBitmap:     %u 次\n", statPackBitmap);
        Serial.println("==========================================\n");
        
        statFindCharIndex = 0;
        statReadPixData = 0;
        statGetGlyphBitmap = 0;
        statDecodePixel = 0;
        statPackBitmap = 0;
        LvZhFont::resetStats();
        statLastPrint = millis();
    }
}

int XFontAdapter::findCharIndex(uint32_t unicode) {
    statFindCharIndex++;
    if (unicode > 0xFFFF) return -1;
    return pgm_read_word(&charIndexMap[unicode]);
}

bool XFontAdapter::readPixData(int charIndex) {
    statReadPixData++;
    checkFileOpen();
    if (!fileOpen) return false;
    
    int pixBeginIdx = unicodeBeginIdx + charIndex * fontPage;
    fontFile.seek(pixBeginIdx);
    fontFile.read(pixBuf, fontPage);
    
    return true;
}

bool XFontAdapter::getGlyphBitmap(uint32_t unicode, uint8_t* bitmap, int* width, int* height) {
    if (!initialized) return false;
    
    statGetGlyphBitmap++;
    
    bool isAscii = (unicode <= 127);
    *width = isAscii ? fontSize / 2 : fontSize;
    *height = fontSize;
    
    int bitmapSize = (*width) * (*height);
    
    int charIndex = findCharIndex(unicode);
    
    if (charIndex < 0) {
        memset(bitmap, 0, bitmapSize);
        return true;
    }
    
    if (!readPixData(charIndex)) {
        return false;
    }
    
    memset(tempBitmap, 0, fontSize * fontSize);
    
    int bitIdx = 0;
    for (int i = 0; i < fontPage; i++) {
        const char* p = strchr(s64, pixBuf[i]);
        if (p) {
            int d = p - s64;
            for (int k = 5; k >= 0; k--) {
                statDecodePixel++;
                int pixel = (d >> k) & 1;
                int x = bitIdx % fontSize;
                int y = bitIdx / fontSize;
                if (y < fontSize && x < fontSize) {
                    tempBitmap[y * fontSize + x] = pixel;
                }
                bitIdx++;
            }
        }
    }
    
    memset(bitmap, 0, bitmapSize);
    for (int y = 0; y < *height; y++) {
        for (int x = 0; x < *width; x++) {
            bitmap[y * (*width) + x] = tempBitmap[y * fontSize + x];
        }
    }
    
    return true;
}
