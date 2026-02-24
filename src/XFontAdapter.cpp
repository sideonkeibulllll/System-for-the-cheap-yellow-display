#include "XFontAdapter.h"
#include "font_index_map.h"

const char* XFontAdapter::s64 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#*$";
uint8_t XFontAdapter::pixBuf[128];
uint8_t XFontAdapter::tempBitmap[576];

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
}

int XFontAdapter::findCharIndex(uint32_t unicode) {
    if (unicode > 0xFFFF) return -1;
    return pgm_read_word(&charIndexMap[unicode]);
}

bool XFontAdapter::readPixData(int charIndex) {
    checkFileOpen();
    if (!fileOpen) return false;
    
    int pixBeginIdx = unicodeBeginIdx + charIndex * fontPage;
    fontFile.seek(pixBeginIdx);
    fontFile.read(pixBuf, fontPage);
    
    return true;
}

bool XFontAdapter::getGlyphBitmap(uint32_t unicode, uint8_t* bitmap, int* width, int* height) {
    if (!initialized) return false;
    
    bool isAscii = (unicode <= 127);
    *width = isAscii ? fontSize / 2 : fontSize;
    *height = fontSize;
    
    int bitmapSize = (*width) * (*height);
    
    static uint32_t cachedUnicode = 0xFFFFFFFF;
    static uint8_t cachedBitmap[576];
    static int cachedWidth = 0;
    static int cachedHeight = 0;
    
    if (unicode == cachedUnicode && cachedWidth == *width && cachedHeight == *height) {
        memcpy(bitmap, cachedBitmap, bitmapSize);
        return true;
    }
    
    int charIndex = findCharIndex(unicode);
    
    if (charIndex < 0) {
        memset(bitmap, 0, bitmapSize);
        cachedUnicode = unicode;
        cachedWidth = *width;
        cachedHeight = *height;
        memcpy(cachedBitmap, bitmap, bitmapSize);
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
    
    cachedUnicode = unicode;
    cachedWidth = *width;
    cachedHeight = *height;
    memcpy(cachedBitmap, bitmap, bitmapSize);
    
    return true;
}
