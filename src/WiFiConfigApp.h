#ifndef WIFICONFIGAPP_H
#define WIFICONFIGAPP_H

#include "AppManager.h"
#include <WiFi.h>

#define WIFI_MAX_SCAN_RESULTS   6
#define WIFI_MAX_SSID_LEN       20
#define WIFI_MAX_PASS_LEN       32
#define WIFI_SCAN_TIMEOUT_MS    15000
#define WIFI_CONNECT_TIMEOUT_MS 20000
#define WIFI_AP_SSID_PREFIX     "ESP_"

typedef enum {
    WIFI_MODE_IDLE = 0,
    WIFI_MODE_SCANNING,
    WIFI_MODE_CONNECTING,
    WIFI_MODE_AP_CONFIG,
    WIFI_MODE_CONNECTED,
    WIFI_MODE_FAILED
} wifi_config_mode_t;

typedef struct {
    char ssid[WIFI_MAX_SSID_LEN];
    int32_t rssi;
    wifi_auth_mode_t encryption;
    bool isOpen;
} wifi_scan_result_t;

class WiFiConfigApp : public BaseApp {
private:
    lv_obj_t* btnBack;
    lv_obj_t* btnScan;
    lv_obj_t* btnAPMode;
    lv_obj_t* labelStatus;
    lv_obj_t* listNetworks;
    lv_obj_t* keyboard;
    lv_obj_t* textareaPassword;
    lv_obj_t* btnConnect;
    lv_obj_t* btnCancel;
    lv_obj_t* qrCanvas;
    lv_obj_t* labelQRHint;
    lv_obj_t* labelAPInfo;
    
    wifi_config_mode_t _mode;
    wifi_scan_result_t _scanResults[WIFI_MAX_SCAN_RESULTS];
    int _scanCount;
    int _selectedIndex;
    char _selectedSSID[WIFI_MAX_SSID_LEN];
    char _password[WIFI_MAX_PASS_LEN];
    
    bool _apModeActive;
    char _apSSID[24];
    char _apPassword[16];
    
    uint32_t _lastUpdateMs;
    bool _connecting;
    int _connectAttempts;
    
    static void btn_scan_cb(lv_event_t* e);
    static void btn_ap_mode_cb(lv_event_t* e);
    static void btn_back_cb(lv_event_t* e);
    static void btn_connect_cb(lv_event_t* e);
    static void btn_cancel_cb(lv_event_t* e);
    static void list_select_cb(lv_event_t* e);
    static void keyboard_event_cb(lv_event_t* e);
    
    void startScan();
    void handleScanComplete();
    void showPasswordDialog(const char* ssid);
    void hidePasswordDialog();
    void startConnect(const char* ssid, const char* password);
    void startAPMode();
    void stopAPMode();
    void drawQRCode(const char* data);
    void updateStatus(const char* text);
    void updateStatus(const String& text);
    void clearNetworkList();
    void populateNetworkList();
    void showMainUI();
    void hideMainUI();
    void showQRUI();
    void hideQRUI();
    const char* getEncryptionType(wifi_auth_mode_t enc);
    void saveWiFiConfig(const char* ssid, const char* password);
    bool loadWiFiConfig();
    
protected:
    virtual bool createUI() override;
    virtual void destroyUI() override;
    
public:
    WiFiConfigApp();
    virtual ~WiFiConfigApp();
    
    virtual void onUpdate() override;
    virtual app_info_t getInfo() const override;
};

BaseApp* createWiFiConfigApp();

#endif
