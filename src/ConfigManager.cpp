#include "ConfigManager.h"

ConfigManager Config;

ConfigManager::ConfigManager() {
    sdCardAvailable = false;
    configLoaded = false;
}

ConfigManager::~ConfigManager() {
    preferences.end();
}

void ConfigManager::loadDefaults() {
    displayConfig.width = 240;
    displayConfig.height = 320;
    displayConfig.colorDepth = 16;
    displayConfig.vdbBufferSizeKB = 30;
    displayConfig.spiMosi = 13;
    displayConfig.spiMiso = 12;
    displayConfig.spiClk = 14;
    displayConfig.spiCs = 15;
    displayConfig.pinDc = 2;
    displayConfig.pinRst = -1;
    displayConfig.pinBl = 21;
    displayConfig.invertColor = false;
    displayConfig.backlightMaxLevel = 255;
    displayConfig.spiFrequency = 40000000;
    
    touchConfig.spiMosi = 32;
    touchConfig.spiMiso = 39;
    touchConfig.spiClk = 25;
    touchConfig.spiCs = 33;
    touchConfig.pinIrq = 36;
    touchConfig.calibrationMatrix[0] = 1.0f;
    touchConfig.calibrationMatrix[1] = 0.0f;
    touchConfig.calibrationMatrix[2] = 0.0f;
    touchConfig.calibrationMatrix[3] = 0.0f;
    touchConfig.calibrationMatrix[4] = 1.0f;
    touchConfig.calibrationMatrix[5] = 0.0f;
    
    strcpy(storageConfig.sdMountPoint, "S");
    storageConfig.sdSpiMosi = 23;
    storageConfig.sdSpiMiso = 19;
    storageConfig.sdSpiClk = 18;
    storageConfig.sdSpiCs = 5;
    
    strcpy(systemConfig.defaultTheme, "dark");
    strcpy(systemConfig.defaultFont, "F:/fonts/default_16.bin");
    strcpy(systemConfig.logLevel, "INFO");
    systemConfig.idleTimeoutSec = 300;
    systemConfig.wifiAutoConnect = false;
    systemConfig.btEnabled = false;
    
    strcpy(wifiConfig.ssid, "");
    strcpy(wifiConfig.password, "");
}

void ConfigManager::loadFromNVS() {
    preferences.begin("display", true);
    displayConfig.width = preferences.getUShort("width", displayConfig.width);
    displayConfig.height = preferences.getUShort("height", displayConfig.height);
    displayConfig.colorDepth = preferences.getUChar("colorDepth", displayConfig.colorDepth);
    displayConfig.vdbBufferSizeKB = preferences.getUShort("vdbSize", displayConfig.vdbBufferSizeKB);
    displayConfig.spiMosi = preferences.getChar("spiMosi", displayConfig.spiMosi);
    displayConfig.spiMiso = preferences.getChar("spiMiso", displayConfig.spiMiso);
    displayConfig.spiClk = preferences.getChar("spiClk", displayConfig.spiClk);
    displayConfig.spiCs = preferences.getChar("spiCs", displayConfig.spiCs);
    displayConfig.pinDc = preferences.getChar("pinDc", displayConfig.pinDc);
    displayConfig.pinRst = preferences.getChar("pinRst", displayConfig.pinRst);
    displayConfig.pinBl = preferences.getChar("pinBl", displayConfig.pinBl);
    displayConfig.invertColor = preferences.getBool("invertColor", displayConfig.invertColor);
    displayConfig.backlightMaxLevel = preferences.getUChar("blMaxLevel", displayConfig.backlightMaxLevel);
    displayConfig.spiFrequency = preferences.getUInt("spiFreq", displayConfig.spiFrequency);
    preferences.end();
    
    preferences.begin("touch", true);
    touchConfig.spiMosi = preferences.getChar("spiMosi", touchConfig.spiMosi);
    touchConfig.spiMiso = preferences.getChar("spiMiso", touchConfig.spiMiso);
    touchConfig.spiClk = preferences.getChar("spiClk", touchConfig.spiClk);
    touchConfig.spiCs = preferences.getChar("spiCs", touchConfig.spiCs);
    touchConfig.pinIrq = preferences.getChar("pinIrq", touchConfig.pinIrq);
    String calibStr = preferences.getString("calibMatrix", "1,0,0,0,1,0");
    sscanf(calibStr.c_str(), "%f,%f,%f,%f,%f,%f",
           &touchConfig.calibrationMatrix[0], &touchConfig.calibrationMatrix[1],
           &touchConfig.calibrationMatrix[2], &touchConfig.calibrationMatrix[3],
           &touchConfig.calibrationMatrix[4], &touchConfig.calibrationMatrix[5]);
    preferences.end();
    
    preferences.begin("storage", true);
    strncpy(storageConfig.sdMountPoint, 
            preferences.getString("sdMount", storageConfig.sdMountPoint).c_str(), 
            sizeof(storageConfig.sdMountPoint) - 1);
    storageConfig.sdSpiMosi = preferences.getChar("sdMosi", storageConfig.sdSpiMosi);
    storageConfig.sdSpiMiso = preferences.getChar("sdMiso", storageConfig.sdSpiMiso);
    storageConfig.sdSpiClk = preferences.getChar("sdClk", storageConfig.sdSpiClk);
    storageConfig.sdSpiCs = preferences.getChar("sdCs", storageConfig.sdSpiCs);
    preferences.end();
    
    preferences.begin("system", true);
    strncpy(systemConfig.defaultTheme,
            preferences.getString("theme", systemConfig.defaultTheme).c_str(),
            sizeof(systemConfig.defaultTheme) - 1);
    strncpy(systemConfig.defaultFont,
            preferences.getString("font", systemConfig.defaultFont).c_str(),
            sizeof(systemConfig.defaultFont) - 1);
    strncpy(systemConfig.logLevel,
            preferences.getString("logLevel", systemConfig.logLevel).c_str(),
            sizeof(systemConfig.logLevel) - 1);
    systemConfig.idleTimeoutSec = preferences.getUShort("idleTimeout", systemConfig.idleTimeoutSec);
    systemConfig.wifiAutoConnect = preferences.getBool("wifiAuto", systemConfig.wifiAutoConnect);
    systemConfig.btEnabled = preferences.getBool("btEnabled", systemConfig.btEnabled);
    preferences.end();
    
    preferences.begin("wifi", true);
    strncpy(wifiConfig.ssid,
            preferences.getString("ssid", wifiConfig.ssid).c_str(),
            sizeof(wifiConfig.ssid) - 1);
    strncpy(wifiConfig.password,
            preferences.getString("password", wifiConfig.password).c_str(),
            sizeof(wifiConfig.password) - 1);
    preferences.end();
}

bool ConfigManager::loadFromSDCard() {
    if (!sdCardAvailable) {
        return false;
    }
    
    File configFile = SD.open(CONFIG_FILE_PATH);
    if (!configFile) {
        Serial.println("[ConfigManager] No config file found on SD card");
        return false;
    }
    
    Serial.println("[ConfigManager] Loading config from SD card...");
    
    size_t fileSize = configFile.size();
    if (fileSize > 4096) {
        Serial.println("[ConfigManager] Config file too large");
        configFile.close();
        return false;
    }
    
    char* jsonContent = new char[fileSize + 1];
    configFile.readBytes(jsonContent, fileSize);
    jsonContent[fileSize] = '\0';
    configFile.close();
    
    bool result = parseJsonConfig(jsonContent);
    delete[] jsonContent;
    
    return result;
}

bool ConfigManager::parseJsonConfig(const char* jsonContent) {
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    
    if (error) {
        Serial.printf("[ConfigManager] JSON parse error: %s\n", error.c_str());
        return false;
    }
    
    if (doc.containsKey("display")) {
        JsonObject display = doc["display"];
        if (display.containsKey("width")) displayConfig.width = display["width"];
        if (display.containsKey("height")) displayConfig.height = display["height"];
        if (display.containsKey("colorDepth")) displayConfig.colorDepth = display["colorDepth"];
        if (display.containsKey("vdbBufferSizeKB")) displayConfig.vdbBufferSizeKB = display["vdbBufferSizeKB"];
        if (display.containsKey("spiMosi")) displayConfig.spiMosi = display["spiMosi"];
        if (display.containsKey("spiMiso")) displayConfig.spiMiso = display["spiMiso"];
        if (display.containsKey("spiClk")) displayConfig.spiClk = display["spiClk"];
        if (display.containsKey("spiCs")) displayConfig.spiCs = display["spiCs"];
        if (display.containsKey("pinDc")) displayConfig.pinDc = display["pinDc"];
        if (display.containsKey("pinRst")) displayConfig.pinRst = display["pinRst"];
        if (display.containsKey("pinBl")) displayConfig.pinBl = display["pinBl"];
        if (display.containsKey("invertColor")) displayConfig.invertColor = display["invertColor"];
        if (display.containsKey("backlightMaxLevel")) displayConfig.backlightMaxLevel = display["backlightMaxLevel"];
        if (display.containsKey("spiFrequency")) displayConfig.spiFrequency = display["spiFrequency"];
    }
    
    if (doc.containsKey("touch")) {
        JsonObject touch = doc["touch"];
        if (touch.containsKey("spiMosi")) touchConfig.spiMosi = touch["spiMosi"];
        if (touch.containsKey("spiMiso")) touchConfig.spiMiso = touch["spiMiso"];
        if (touch.containsKey("spiClk")) touchConfig.spiClk = touch["spiClk"];
        if (touch.containsKey("spiCs")) touchConfig.spiCs = touch["spiCs"];
        if (touch.containsKey("pinIrq")) touchConfig.pinIrq = touch["pinIrq"];
        if (touch.containsKey("calibrationMatrix")) {
            JsonArray calib = touch["calibrationMatrix"];
            for (int i = 0; i < 6 && i < calib.size(); i++) {
                touchConfig.calibrationMatrix[i] = calib[i];
            }
        }
    }
    
    if (doc.containsKey("storage")) {
        JsonObject storage = doc["storage"];
        if (storage.containsKey("sdMountPoint")) 
            strncpy(storageConfig.sdMountPoint, storage["sdMountPoint"], sizeof(storageConfig.sdMountPoint) - 1);
        if (storage.containsKey("sdSpiMosi")) storageConfig.sdSpiMosi = storage["sdSpiMosi"];
        if (storage.containsKey("sdSpiMiso")) storageConfig.sdSpiMiso = storage["sdSpiMiso"];
        if (storage.containsKey("sdSpiClk")) storageConfig.sdSpiClk = storage["sdSpiClk"];
        if (storage.containsKey("sdSpiCs")) storageConfig.sdSpiCs = storage["sdSpiCs"];
    }
    
    if (doc.containsKey("system")) {
        JsonObject system = doc["system"];
        if (system.containsKey("defaultTheme"))
            strncpy(systemConfig.defaultTheme, system["defaultTheme"], sizeof(systemConfig.defaultTheme) - 1);
        if (system.containsKey("defaultFont"))
            strncpy(systemConfig.defaultFont, system["defaultFont"], sizeof(systemConfig.defaultFont) - 1);
        if (system.containsKey("logLevel"))
            strncpy(systemConfig.logLevel, system["logLevel"], sizeof(systemConfig.logLevel) - 1);
        if (system.containsKey("idleTimeoutSec")) systemConfig.idleTimeoutSec = system["idleTimeoutSec"];
        if (system.containsKey("wifiAutoConnect")) systemConfig.wifiAutoConnect = system["wifiAutoConnect"];
        if (system.containsKey("btEnabled")) systemConfig.btEnabled = system["btEnabled"];
    }
    
    if (doc.containsKey("wifi")) {
        JsonObject wifi = doc["wifi"];
        if (wifi.containsKey("ssid"))
            strncpy(wifiConfig.ssid, wifi["ssid"], sizeof(wifiConfig.ssid) - 1);
        if (wifi.containsKey("password"))
            strncpy(wifiConfig.password, wifi["password"], sizeof(wifiConfig.password) - 1);
    }
    
    Serial.println("[ConfigManager] JSON config loaded successfully");
    return true;
}

bool ConfigManager::begin() {
    Serial.println("[ConfigManager] Initializing...");
    
    loadDefaults();
    Serial.println("[ConfigManager] Default values loaded");
    
    loadFromNVS();
    Serial.println("[ConfigManager] NVS config loaded");
    
    loadFromSDCard();
    
    configLoaded = true;
    Serial.println("[ConfigManager] Initialization complete");
    
    return true;
}

bool ConfigManager::reload() {
    loadDefaults();
    loadFromNVS();
    return loadFromSDCard();
}

void ConfigManager::saveDisplayToNVS() {
    preferences.begin("display", false);
    preferences.putUShort("width", displayConfig.width);
    preferences.putUShort("height", displayConfig.height);
    preferences.putUChar("colorDepth", displayConfig.colorDepth);
    preferences.putUShort("vdbSize", displayConfig.vdbBufferSizeKB);
    preferences.putChar("spiMosi", displayConfig.spiMosi);
    preferences.putChar("spiMiso", displayConfig.spiMiso);
    preferences.putChar("spiClk", displayConfig.spiClk);
    preferences.putChar("spiCs", displayConfig.spiCs);
    preferences.putChar("pinDc", displayConfig.pinDc);
    preferences.putChar("pinRst", displayConfig.pinRst);
    preferences.putChar("pinBl", displayConfig.pinBl);
    preferences.putBool("invertColor", displayConfig.invertColor);
    preferences.putUChar("blMaxLevel", displayConfig.backlightMaxLevel);
    preferences.putUInt("spiFreq", displayConfig.spiFrequency);
    preferences.end();
}

void ConfigManager::saveTouchToNVS() {
    char calibStr[64];
    snprintf(calibStr, sizeof(calibStr), "%.4f,%.4f,%.4f,%.4f,%.4f,%.4f",
             touchConfig.calibrationMatrix[0], touchConfig.calibrationMatrix[1],
             touchConfig.calibrationMatrix[2], touchConfig.calibrationMatrix[3],
             touchConfig.calibrationMatrix[4], touchConfig.calibrationMatrix[5]);
    
    preferences.begin("touch", false);
    preferences.putChar("spiMosi", touchConfig.spiMosi);
    preferences.putChar("spiMiso", touchConfig.spiMiso);
    preferences.putChar("spiClk", touchConfig.spiClk);
    preferences.putChar("spiCs", touchConfig.spiCs);
    preferences.putChar("pinIrq", touchConfig.pinIrq);
    preferences.putString("calibMatrix", calibStr);
    preferences.end();
}

void ConfigManager::saveStorageToNVS() {
    preferences.begin("storage", false);
    preferences.putString("sdMount", storageConfig.sdMountPoint);
    preferences.putChar("sdMosi", storageConfig.sdSpiMosi);
    preferences.putChar("sdMiso", storageConfig.sdSpiMiso);
    preferences.putChar("sdClk", storageConfig.sdSpiClk);
    preferences.putChar("sdCs", storageConfig.sdSpiCs);
    preferences.end();
}

void ConfigManager::saveSystemToNVS() {
    preferences.begin("system", false);
    preferences.putString("theme", systemConfig.defaultTheme);
    preferences.putString("font", systemConfig.defaultFont);
    preferences.putString("logLevel", systemConfig.logLevel);
    preferences.putUShort("idleTimeout", systemConfig.idleTimeoutSec);
    preferences.putBool("wifiAuto", systemConfig.wifiAutoConnect);
    preferences.putBool("btEnabled", systemConfig.btEnabled);
    preferences.end();
}

void ConfigManager::saveWiFiToNVS() {
    preferences.begin("wifi", false);
    preferences.putString("ssid", wifiConfig.ssid);
    preferences.putString("password", wifiConfig.password);
    preferences.end();
}

void ConfigManager::saveAllToNVS() {
    saveDisplayToNVS();
    saveTouchToNVS();
    saveStorageToNVS();
    saveSystemToNVS();
    saveWiFiToNVS();
    Serial.println("[ConfigManager] All config saved to NVS");
}

int ConfigManager::getInt(const char* section, const char* key) {
    String sectionStr = String(section);
    String keyStr = String(key);
    
    if (sectionStr == "display") {
        if (keyStr == "width") return displayConfig.width;
        if (keyStr == "height") return displayConfig.height;
        if (keyStr == "colorDepth") return displayConfig.colorDepth;
        if (keyStr == "vdbBufferSizeKB") return displayConfig.vdbBufferSizeKB;
        if (keyStr == "spiMosi") return displayConfig.spiMosi;
        if (keyStr == "spiMiso") return displayConfig.spiMiso;
        if (keyStr == "spiClk") return displayConfig.spiClk;
        if (keyStr == "spiCs") return displayConfig.spiCs;
        if (keyStr == "pinDc") return displayConfig.pinDc;
        if (keyStr == "pinRst") return displayConfig.pinRst;
        if (keyStr == "pinBl") return displayConfig.pinBl;
        if (keyStr == "backlightMaxLevel") return displayConfig.backlightMaxLevel;
        if (keyStr == "spiFrequency") return displayConfig.spiFrequency;
    }
    else if (sectionStr == "touch") {
        if (keyStr == "spiMosi") return touchConfig.spiMosi;
        if (keyStr == "spiMiso") return touchConfig.spiMiso;
        if (keyStr == "spiClk") return touchConfig.spiClk;
        if (keyStr == "spiCs") return touchConfig.spiCs;
        if (keyStr == "pinIrq") return touchConfig.pinIrq;
    }
    else if (sectionStr == "storage") {
        if (keyStr == "sdSpiMosi") return storageConfig.sdSpiMosi;
        if (keyStr == "sdSpiMiso") return storageConfig.sdSpiMiso;
        if (keyStr == "sdSpiClk") return storageConfig.sdSpiClk;
        if (keyStr == "sdSpiCs") return storageConfig.sdSpiCs;
    }
    else if (sectionStr == "system") {
        if (keyStr == "idleTimeoutSec") return systemConfig.idleTimeoutSec;
        if (keyStr == "wifiAutoConnect") return systemConfig.wifiAutoConnect ? 1 : 0;
        if (keyStr == "btEnabled") return systemConfig.btEnabled ? 1 : 0;
    }
    
    return 0;
}

float ConfigManager::getFloat(const char* section, const char* key) {
    String sectionStr = String(section);
    String keyStr = String(key);
    
    if (sectionStr == "touch") {
        if (keyStr.startsWith("calibMatrix_")) {
            int index = keyStr.substring(12).toInt();
            if (index >= 0 && index < 6) {
                return touchConfig.calibrationMatrix[index];
            }
        }
    }
    
    return 0.0f;
}

String ConfigManager::getString(const char* section, const char* key) {
    String sectionStr = String(section);
    String keyStr = String(key);
    
    if (sectionStr == "storage") {
        if (keyStr == "sdMountPoint") return String(storageConfig.sdMountPoint);
    }
    else if (sectionStr == "system") {
        if (keyStr == "defaultTheme") return String(systemConfig.defaultTheme);
        if (keyStr == "defaultFont") return String(systemConfig.defaultFont);
        if (keyStr == "logLevel") return String(systemConfig.logLevel);
    }
    else if (sectionStr == "wifi") {
        if (keyStr == "ssid") return String(wifiConfig.ssid);
        if (keyStr == "password") return String(wifiConfig.password);
    }
    
    return "";
}

bool ConfigManager::getBool(const char* section, const char* key) {
    String sectionStr = String(section);
    String keyStr = String(key);
    
    if (sectionStr == "display") {
        if (keyStr == "invertColor") return displayConfig.invertColor;
    }
    else if (sectionStr == "system") {
        if (keyStr == "wifiAutoConnect") return systemConfig.wifiAutoConnect;
        if (keyStr == "btEnabled") return systemConfig.btEnabled;
    }
    
    return false;
}

void ConfigManager::setInt(const char* section, const char* key, int value) {
    String sectionStr = String(section);
    String keyStr = String(key);
    
    if (sectionStr == "display") {
        if (keyStr == "width") displayConfig.width = value;
        else if (keyStr == "height") displayConfig.height = value;
        else if (keyStr == "colorDepth") displayConfig.colorDepth = value;
        else if (keyStr == "vdbBufferSizeKB") displayConfig.vdbBufferSizeKB = value;
        else if (keyStr == "spiMosi") displayConfig.spiMosi = value;
        else if (keyStr == "spiMiso") displayConfig.spiMiso = value;
        else if (keyStr == "spiClk") displayConfig.spiClk = value;
        else if (keyStr == "spiCs") displayConfig.spiCs = value;
        else if (keyStr == "pinDc") displayConfig.pinDc = value;
        else if (keyStr == "pinRst") displayConfig.pinRst = value;
        else if (keyStr == "pinBl") displayConfig.pinBl = value;
        else if (keyStr == "backlightMaxLevel") displayConfig.backlightMaxLevel = value;
        else if (keyStr == "spiFrequency") displayConfig.spiFrequency = value;
    }
    else if (sectionStr == "touch") {
        if (keyStr == "spiMosi") touchConfig.spiMosi = value;
        else if (keyStr == "spiMiso") touchConfig.spiMiso = value;
        else if (keyStr == "spiClk") touchConfig.spiClk = value;
        else if (keyStr == "spiCs") touchConfig.spiCs = value;
        else if (keyStr == "pinIrq") touchConfig.pinIrq = value;
    }
    else if (sectionStr == "storage") {
        if (keyStr == "sdSpiMosi") storageConfig.sdSpiMosi = value;
        else if (keyStr == "sdSpiMiso") storageConfig.sdSpiMiso = value;
        else if (keyStr == "sdSpiClk") storageConfig.sdSpiClk = value;
        else if (keyStr == "sdSpiCs") storageConfig.sdSpiCs = value;
    }
    else if (sectionStr == "system") {
        if (keyStr == "idleTimeoutSec") systemConfig.idleTimeoutSec = value;
        else if (keyStr == "wifiAutoConnect") systemConfig.wifiAutoConnect = (value != 0);
        else if (keyStr == "btEnabled") systemConfig.btEnabled = (value != 0);
    }
}

void ConfigManager::setFloat(const char* section, const char* key, float value) {
    String sectionStr = String(section);
    String keyStr = String(key);
    
    if (sectionStr == "touch") {
        if (keyStr.startsWith("calibMatrix_")) {
            int index = keyStr.substring(12).toInt();
            if (index >= 0 && index < 6) {
                touchConfig.calibrationMatrix[index] = value;
            }
        }
    }
}

void ConfigManager::setString(const char* section, const char* key, const char* value) {
    String sectionStr = String(section);
    String keyStr = String(key);
    
    if (sectionStr == "storage") {
        if (keyStr == "sdMountPoint") 
            strncpy(storageConfig.sdMountPoint, value, sizeof(storageConfig.sdMountPoint) - 1);
    }
    else if (sectionStr == "system") {
        if (keyStr == "defaultTheme")
            strncpy(systemConfig.defaultTheme, value, sizeof(systemConfig.defaultTheme) - 1);
        else if (keyStr == "defaultFont")
            strncpy(systemConfig.defaultFont, value, sizeof(systemConfig.defaultFont) - 1);
        else if (keyStr == "logLevel")
            strncpy(systemConfig.logLevel, value, sizeof(systemConfig.logLevel) - 1);
    }
    else if (sectionStr == "wifi") {
        if (keyStr == "ssid")
            strncpy(wifiConfig.ssid, value, sizeof(wifiConfig.ssid) - 1);
        else if (keyStr == "password")
            strncpy(wifiConfig.password, value, sizeof(wifiConfig.password) - 1);
    }
}

void ConfigManager::setBool(const char* section, const char* key, bool value) {
    String sectionStr = String(section);
    String keyStr = String(key);
    
    if (sectionStr == "display") {
        if (keyStr == "invertColor") displayConfig.invertColor = value;
    }
    else if (sectionStr == "system") {
        if (keyStr == "wifiAutoConnect") systemConfig.wifiAutoConnect = value;
        else if (keyStr == "btEnabled") systemConfig.btEnabled = value;
    }
}

void ConfigManager::printConfig() {
    Serial.println("\n========================================");
    Serial.println("  Current Configuration");
    Serial.println("========================================");
    
    Serial.println("\n[Display]");
    Serial.printf("  Resolution: %d x %d\n", displayConfig.width, displayConfig.height);
    Serial.printf("  Color Depth: %d bits\n", displayConfig.colorDepth);
    Serial.printf("  VDB Buffer Size: %d KB\n", displayConfig.vdbBufferSizeKB);
    Serial.printf("  SPI Pins: MOSI=%d, MISO=%d, CLK=%d, CS=%d\n", 
                  displayConfig.spiMosi, displayConfig.spiMiso, displayConfig.spiClk, displayConfig.spiCs);
    Serial.printf("  Control Pins: DC=%d, RST=%d, BL=%d\n", 
                  displayConfig.pinDc, displayConfig.pinRst, displayConfig.pinBl);
    Serial.printf("  Invert Color: %s\n", displayConfig.invertColor ? "Yes" : "No");
    Serial.printf("  Backlight Max: %d\n", displayConfig.backlightMaxLevel);
    Serial.printf("  SPI Frequency: %d MHz\n", displayConfig.spiFrequency / 1000000);
    
    Serial.println("\n[Touch]");
    Serial.printf("  SPI Pins: MOSI=%d, MISO=%d, CLK=%d, CS=%d\n",
                  touchConfig.spiMosi, touchConfig.spiMiso, touchConfig.spiClk, touchConfig.spiCs);
    Serial.printf("  IRQ Pin: %d\n", touchConfig.pinIrq);
    Serial.printf("  Calibration Matrix: [%.4f, %.4f, %.4f, %.4f, %.4f, %.4f]\n",
                  touchConfig.calibrationMatrix[0], touchConfig.calibrationMatrix[1],
                  touchConfig.calibrationMatrix[2], touchConfig.calibrationMatrix[3],
                  touchConfig.calibrationMatrix[4], touchConfig.calibrationMatrix[5]);
    
    Serial.println("\n[Storage]");
    Serial.printf("  SD Mount Point: %s\n", storageConfig.sdMountPoint);
    Serial.printf("  SD SPI Pins: MOSI=%d, MISO=%d, CLK=%d, CS=%d\n",
                  storageConfig.sdSpiMosi, storageConfig.sdSpiMiso, 
                  storageConfig.sdSpiClk, storageConfig.sdSpiCs);
    
    Serial.println("\n[System]");
    Serial.printf("  Default Theme: %s\n", systemConfig.defaultTheme);
    Serial.printf("  Default Font: %s\n", systemConfig.defaultFont);
    Serial.printf("  Log Level: %s\n", systemConfig.logLevel);
    Serial.printf("  Idle Timeout: %d seconds\n", systemConfig.idleTimeoutSec);
    Serial.printf("  WiFi Auto Connect: %s\n", systemConfig.wifiAutoConnect ? "Yes" : "No");
    Serial.printf("  Bluetooth Enabled: %s\n", systemConfig.btEnabled ? "Yes" : "No");
    
    Serial.println("\n[WiFi]");
    Serial.printf("  SSID: %s\n", wifiConfig.ssid);
    Serial.printf("  Password: %s\n", strlen(wifiConfig.password) > 0 ? "********" : "(not set)");
    
    Serial.println("\n========================================\n");
}
