#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <SD.h>

#define CONFIG_FILE_PATH "/config.json"
#define MAX_CONFIG_STRING_LENGTH 256

struct DisplayConfig {
    uint16_t width;
    uint16_t height;
    uint8_t colorDepth;
    uint16_t vdbBufferSizeKB;
    int8_t spiMosi;
    int8_t spiMiso;
    int8_t spiClk;
    int8_t spiCs;
    int8_t pinDc;
    int8_t pinRst;
    int8_t pinBl;
    bool invertColor;
    uint8_t backlightMaxLevel;
    uint32_t spiFrequency;
};

struct TouchConfig {
    int8_t spiMosi;
    int8_t spiMiso;
    int8_t spiClk;
    int8_t spiCs;
    int8_t pinIrq;
    float calibrationMatrix[6];
};

struct StorageConfig {
    char sdMountPoint[8];
    int8_t sdSpiMosi;
    int8_t sdSpiMiso;
    int8_t sdSpiClk;
    int8_t sdSpiCs;
};

struct SystemConfig {
    char defaultTheme[32];
    char defaultFont[64];
    char logLevel[16];
    uint16_t idleTimeoutSec;
    bool wifiAutoConnect;
    bool btEnabled;
};

struct WiFiConfig {
    char ssid[64];
    char password[64];
};

class ConfigManager {
private:
    Preferences preferences;
    bool sdCardAvailable;
    bool configLoaded;
    
    DisplayConfig displayConfig;
    TouchConfig touchConfig;
    StorageConfig storageConfig;
    SystemConfig systemConfig;
    WiFiConfig wifiConfig;
    
    void loadDefaults();
    void loadFromNVS();
    bool loadFromSDCard();
    bool parseJsonConfig(const char* jsonContent);
    
    void saveDisplayToNVS();
    void saveTouchToNVS();
    void saveStorageToNVS();
    void saveSystemToNVS();
    void saveWiFiToNVS();
    
public:
    ConfigManager();
    ~ConfigManager();
    
    bool begin();
    bool reload();
    
    int getInt(const char* section, const char* key);
    float getFloat(const char* section, const char* key);
    String getString(const char* section, const char* key);
    bool getBool(const char* section, const char* key);
    
    void setInt(const char* section, const char* key, int value);
    void setFloat(const char* section, const char* key, float value);
    void setString(const char* section, const char* key, const char* value);
    void setBool(const char* section, const char* key, bool value);
    
    DisplayConfig& getDisplayConfig() { return displayConfig; }
    TouchConfig& getTouchConfig() { return touchConfig; }
    StorageConfig& getStorageConfig() { return storageConfig; }
    SystemConfig& getSystemConfig() { return systemConfig; }
    WiFiConfig& getWiFiConfig() { return wifiConfig; }
    
    void saveAllToNVS();
    void printConfig();
    bool isSDCardAvailable() { return sdCardAvailable; }
    bool isConfigLoaded() { return configLoaded; }
};

extern ConfigManager Config;

#endif
