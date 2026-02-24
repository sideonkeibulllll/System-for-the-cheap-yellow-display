#include "XFontAdapter.h"

const char* XFontAdapter::s64 = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@#*$";

XFontAdapter XFontAdapter::instance;

XFontAdapter::XFontAdapter() {
    initialized = false;
    fontSize = 0;
    binType = 0;
    fontPage = 0;
    totalChars = 0;
    unicodeBeginIdx = 0;
    unicodeIndex = nullptr;
    unicodeIndexLen = 0;
    fontPath = "/x.font";
}

XFontAdapter::~XFontAdapter() {
    end();
}

void XFontAdapter::end() {
    if (unicodeIndex) {
        free(unicodeIndex);
        unicodeIndex = nullptr;
    }
    initialized = false;
}

bool XFontAdapter::begin(const char* path) {
    if (initialized) return true;
    
    fontPath = path;
    
    if (!SPIFFS.begin()) {
        Serial.println("[XFontAdapter] SPIFFS mount failed");
        return false;
    }
    
    if (!SPIFFS.exists(fontPath)) {
        Serial.printf("[XFontAdapter] Font file not found: %s\n", fontPath.c_str());
        return false;
    }
    
    File fontFile = SPIFFS.open(fontPath, "r");
    if (!fontFile) {
        Serial.println("[XFontAdapter] Failed to open font file");
        return false;
    }
    
    Serial.printf("[XFontAdapter] Font file opened, size: %d bytes\n", fontFile.size());
    
    uint8_t bufTotalStr[6];
    uint8_t bufFontSize[2];
    uint8_t bufBinType[2];
    
    fontFile.read(bufTotalStr, 6);
    fontFile.read(bufFontSize, 2);
    fontFile.read(bufBinType, 2);
    
    Serial.printf("[XFontAdapter] Header: totalStr=%02x%02x%02x%02x%02x%02x, fontSize=%02x%02x, binType=%02x%02x\n",
                  bufTotalStr[0], bufTotalStr[1], bufTotalStr[2], 
                  bufTotalStr[3], bufTotalStr[4], bufTotalStr[5],
                  bufFontSize[0], bufFontSize[1],
                  bufBinType[0], bufBinType[1]);
    
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
    
    unicodeIndexLen = totalChars * 5;
    
    unicodeBeginIdx = 6 + 2 + 2 + unicodeIndexLen;
    
    fontFile.close();
    
    initialized = true;
    Serial.printf("[XFontAdapter] Initialized: fontSize=%d, totalChars=%d, binType=%d, fontPage=%d, unicodeBeginIdx=%d\n", 
                  fontSize, totalChars, binType, fontPage, unicodeBeginIdx);
    
    return true;
}

int XFontAdapter::findCharIndex(const char* unicodeHex) {
    if (!SPIFFS.exists(fontPath)) return -1;
    
    File f = SPIFFS.open(fontPath, "r");
    if (!f) return -1;
    
    char searchKey[6];
    searchKey[0] = 'u';
    searchKey[1] = unicodeHex[0];
    searchKey[2] = unicodeHex[1];
    searchKey[3] = unicodeHex[2];
    searchKey[4] = unicodeHex[3];
    searchKey[5] = '\0';
    
    f.seek(10);
    
    uint8_t buf[512];
    int charsRead = 0;
    int index = 0;
    
    while (charsRead < totalChars) {
        int toRead = 512;
        int remaining = (totalChars - charsRead) * 5;
        if (remaining < toRead) toRead = remaining;
        
        int bytesRead = f.read(buf, toRead);
        if (bytesRead <= 0) break;
        
        for (int i = 0; i < bytesRead - 4; i += 5) {
            if (buf[i] == searchKey[0] &&
                buf[i+1] == searchKey[1] &&
                buf[i+2] == searchKey[2] &&
                buf[i+3] == searchKey[3] &&
                buf[i+4] == searchKey[4]) {
                f.close();
                return index;
            }
            index++;
        }
        charsRead += bytesRead / 5;
    }
    
    f.close();
    return -1;
}

String XFontAdapter::getPixData(int charIndex) {
    if (!SPIFFS.exists(fontPath)) return "";
    
    File f = SPIFFS.open(fontPath, "r");
    if (!f) return "";
    
    int pixBeginIdx = unicodeBeginIdx + charIndex * fontPage;
    f.seek(pixBeginIdx);
    
    uint8_t* buf = (uint8_t*)malloc(fontPage + 1);
    f.read(buf, fontPage);
    buf[fontPage] = '\0';
    
    String result = String((char*)buf);
    free(buf);
    f.close();
    
    return result;
}

bool XFontAdapter::getGlyphBitmap(uint32_t unicode, uint8_t* bitmap, int* width, int* height) {
    if (!initialized) return false;
    
    char hex[5];
    sprintf(hex, "%04x", unicode);
    
    int charIndex = findCharIndex(hex);
    if (charIndex < 0) {
        *width = fontSize / 2;
        *height = fontSize;
        memset(bitmap, 0, (*width) * (*height));
        return true;
    }
    
    String pixData = getPixData(charIndex);
    if (pixData.length() == 0) return false;
    
    String binData = "";
    for (int i = 0; i < pixData.length(); i++) {
        const char* p = strchr(s64, pixData[i]);
        if (p) {
            int d = p - s64;
            for (int k = 5; k >= 0; k--) {
                binData += ((d >> k) & 1) ? '1' : '0';
            }
        }
    }
    
    bool isAscii = (unicode <= 127);
    *width = isAscii ? fontSize / 2 : fontSize;
    *height = fontSize;
    
    int bitmapSize = (*width) * (*height);
    memset(bitmap, 0, bitmapSize);
    
    for (int y = 0; y < fontSize; y++) {
        for (int x = 0; x < *width; x++) {
            int srcIdx = y * fontSize + x;
            if (srcIdx < binData.length() && binData[srcIdx] == '1') {
                bitmap[y * (*width) + x] = 1;
            }
        }
    }
    
    return true;
}
