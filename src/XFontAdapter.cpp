#include "XFontAdapter.h"
#include "font_index_map.h"
#include "LvZhFont.h"

const char* XFontAdapter::s64 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#*$";
uint8_t XFontAdapter::pixBuf[128];

uint32_t XFontAdapter::cacheUnicode[CACHE_SIZE];
uint8_t XFontAdapter::cacheBitmap[CACHE_SIZE][72];
int XFontAdapter::cacheIndex = 0;

static const uint8_t s64Decode[256] PROGMEM = {
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255, 63, 66,255,255,255,255,255, 64,255,255,255,255,255,
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9,255,255,255,255,255,255,
     62, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
     51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255, 65,255,255,255,
    255, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
     25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};

unsigned long XFontAdapter::statLastPrint = 0;
unsigned int XFontAdapter::statFindCharIndex = 0;
unsigned int XFontAdapter::statReadPixData = 0;
unsigned int XFontAdapter::statGetGlyphBitmap = 0;
unsigned int XFontAdapter::statDecodePixel = 0;
unsigned int XFontAdapter::statCacheHit = 0;

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
    cacheIndex = 0;
    for (int i = 0; i < CACHE_SIZE; i++) {
        cacheUnicode[i] = 0xFFFFFFFF;
    }
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
        Serial.printf("cacheHit:       %u 次\n", statCacheHit);
        Serial.println("--- 像素处理 ---");
        Serial.printf("decodePixel:    %u 次\n", statDecodePixel);
        Serial.println("==========================================\n");
        
        statFindCharIndex = 0;
        statReadPixData = 0;
        statGetGlyphBitmap = 0;
        statDecodePixel = 0;
        statCacheHit = 0;
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

int XFontAdapter::findInCache(uint32_t unicode) {
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cacheUnicode[i] == unicode) {
            return i;
        }
    }
    return -1;
}

void XFontAdapter::addToCache(uint32_t unicode, const uint8_t* bitmap) {
    cacheUnicode[cacheIndex] = unicode;
    memcpy(cacheBitmap[cacheIndex], bitmap, 72);
    cacheIndex = (cacheIndex + 1) % CACHE_SIZE;
}

const uint8_t* XFontAdapter::getGlyphBitmapPacked(uint32_t unicode, int* width, int* height) {
    if (!initialized) return nullptr;
    
    statGetGlyphBitmap++;
    
    bool isAscii = (unicode <= 127);
    *width = isAscii ? fontSize / 2 : fontSize;
    *height = fontSize;
    
    int cacheIdx = findInCache(unicode);
    if (cacheIdx >= 0) {
        statCacheHit++;
        return cacheBitmap[cacheIdx];
    }
    
    static uint8_t packedBitmap[72];
    memset(packedBitmap, 0, sizeof(packedBitmap));
    
    int charIndex = findCharIndex(unicode);
    
    if (charIndex < 0) {
        return packedBitmap;
    }
    
    if (!readPixData(charIndex)) {
        return packedBitmap;
    }
    
    int bitIdx = 0;
    for (int i = 0; i < fontPage; i++) {
        uint8_t c = pixBuf[i];
        uint8_t d = pgm_read_byte(&s64Decode[c]);
        if (d < 64) {
            for (int k = 5; k >= 0; k--) {
                statDecodePixel++;
                int pixel = (d >> k) & 1;
                if (pixel) {
                    int x = bitIdx % fontSize;
                    int y = bitIdx / fontSize;
                    if (y < fontSize && x < *width) {
                        int packedIdx = y * (*width) + x;
                        packedBitmap[packedIdx / 8] |= (1 << (7 - (packedIdx % 8)));
                    }
                }
                bitIdx++;
            }
        }
    }
    
    addToCache(unicode, packedBitmap);
    
    return packedBitmap;
}
