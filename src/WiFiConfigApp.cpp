#include "WiFiConfigApp.h"
#include "BSP.h"
#include "PowerManager.h"
#include "Storage.h"
#include <qrcode.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <SD.h>

WiFiConfigApp::WiFiConfigApp() : BaseApp("WiFiConfig") {
    btnBack = nullptr;
    btnScan = nullptr;
    btnAPMode = nullptr;
    btnLastConnect = nullptr;
    labelStatus = nullptr;
    listNetworks = nullptr;
    keyboard = nullptr;
    textareaPassword = nullptr;
    btnConnect = nullptr;
    btnCancel = nullptr;
    qrCanvas = nullptr;
    labelQRHint = nullptr;
    labelAPInfo = nullptr;
    
    _mode = WIFI_MODE_IDLE;
    _scanCount = 0;
    _selectedIndex = -1;
    _selectedSSID[0] = '\0';
    _password[0] = '\0';
    _apModeActive = false;
    _lastUpdateMs = 0;
    _connecting = false;
    _connectAttempts = 0;
    
    _lastSSID[0] = '\0';
    _lastPassword[0] = '\0';
    _lastConfigValid = false;
    _lastConnectStartMs = 0;
}

WiFiConfigApp::~WiFiConfigApp() {
    if (_apModeActive) {
        stopAPMode();
    }
    // 不再断开WiFi连接，保持连接状态
}

bool WiFiConfigApp::createUI() {
    lv_obj_t* scr = getScreen();
    
    loadWiFiConfigFromSD();
    
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_WIFI " WiFi Config");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    labelStatus = lv_label_create(scr);
    if (WiFi.status() == WL_CONNECTED) {
        lv_label_set_text_fmt(labelStatus, "Connected: %s", WiFi.localIP().toString().c_str());
        lv_obj_set_style_text_color(labelStatus, lv_color_make(0x00, 0xFF, 0x00), 0);
    } else {
        lv_label_set_text(labelStatus, "Ready");
        lv_obj_set_style_text_color(labelStatus, lv_color_make(0xFF, 0xFF, 0x00), 0);
    }
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_12, 0);
    lv_obj_align(labelStatus, LV_ALIGN_TOP_MID, 0, 30);
    
    btnScan = lv_btn_create(scr);
    lv_obj_set_size(btnScan, 100, 30);
    lv_obj_align(btnScan, LV_ALIGN_TOP_LEFT, 10, 55);
    lv_obj_add_event_cb(btnScan, btn_scan_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnScan, lv_color_make(0x00, 0x60, 0x80), 0);
    lv_obj_t* labelScan = lv_label_create(btnScan);
    lv_label_set_text(labelScan, LV_SYMBOL_REFRESH " Scan");
    lv_obj_set_style_text_font(labelScan, &lv_font_montserrat_12, 0);
    lv_obj_center(labelScan);
    
    btnLastConnect = lv_btn_create(scr);
    lv_obj_set_size(btnLastConnect, 100, 30);
    lv_obj_align(btnLastConnect, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_add_event_cb(btnLastConnect, btn_last_connect_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnLastConnect, lv_color_make(0x00, 0x80, 0x00), 0);
    lv_obj_t* labelLast = lv_label_create(btnLastConnect);
    lv_label_set_text(labelLast, LV_SYMBOL_LOOP " Last");
    lv_obj_set_style_text_font(labelLast, &lv_font_montserrat_12, 0);
    lv_obj_center(labelLast);
    updateLastConnectButton();
    
    btnAPMode = lv_btn_create(scr);
    lv_obj_set_size(btnAPMode, 100, 30);
    lv_obj_align(btnAPMode, LV_ALIGN_TOP_RIGHT, -10, 55);
    lv_obj_add_event_cb(btnAPMode, btn_ap_mode_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnAPMode, lv_color_make(0x80, 0x40, 0x00), 0);
    lv_obj_t* labelAP = lv_label_create(btnAPMode);
    lv_label_set_text(labelAP, LV_SYMBOL_KEYBOARD " QR");
    lv_obj_set_style_text_font(labelAP, &lv_font_montserrat_12, 0);
    lv_obj_center(labelAP);
    
    listNetworks = lv_list_create(scr);
    lv_obj_set_size(listNetworks, BSP_DISPLAY_WIDTH - 20, 110);
    lv_obj_align(listNetworks, LV_ALIGN_TOP_MID, 0, 95);
    lv_obj_set_style_bg_color(listNetworks, lv_color_make(0x20, 0x20, 0x20), 0);
    
    btnBack = lv_btn_create(scr);
    lv_obj_set_size(btnBack, 80, 35);
    lv_obj_align(btnBack, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_event_cb(btnBack, btn_back_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnBack, lv_color_make(0x40, 0x40, 0x80), 0);
    lv_obj_t* labelBack = lv_label_create(btnBack);
    lv_label_set_text(labelBack, LV_SYMBOL_LEFT " Back");
    lv_obj_set_style_text_font(labelBack, &lv_font_montserrat_14, 0);
    lv_obj_center(labelBack);
    
    return true;
}

void WiFiConfigApp::destroyUI() {
    if (keyboard) {
        lv_obj_del(keyboard);
        keyboard = nullptr;
    }
    if (textareaPassword) {
        lv_obj_del(textareaPassword);
        textareaPassword = nullptr;
    }
    if (btnConnect) {
        lv_obj_del(btnConnect);
        btnConnect = nullptr;
    }
    if (btnCancel) {
        lv_obj_del(btnCancel);
        btnCancel = nullptr;
    }
    if (qrCanvas) {
        lv_obj_del(qrCanvas);
        qrCanvas = nullptr;
    }
    if (labelQRHint) {
        lv_obj_del(labelQRHint);
        labelQRHint = nullptr;
    }
    if (labelAPInfo) {
        lv_obj_del(labelAPInfo);
        labelAPInfo = nullptr;
    }
}

void WiFiConfigApp::btn_scan_cb(lv_event_t* e) {
    WiFiConfigApp* app = (WiFiConfigApp*)lv_event_get_user_data(e);
    app->startScan();
}

void WiFiConfigApp::btn_ap_mode_cb(lv_event_t* e) {
    WiFiConfigApp* app = (WiFiConfigApp*)lv_event_get_user_data(e);
    app->startAPMode();
}

void WiFiConfigApp::btn_last_connect_cb(lv_event_t* e) {
    WiFiConfigApp* app = (WiFiConfigApp*)lv_event_get_user_data(e);
    app->startLastConnect();
}

void WiFiConfigApp::btn_back_cb(lv_event_t* e) {
    WiFiConfigApp* app = (WiFiConfigApp*)lv_event_get_user_data(e);
    
    if (app->_apModeActive) {
        app->stopAPMode();
    }
    if (app->_mode == WIFI_MODE_CONNECTING) {
        WiFi.disconnect();
        app->_connecting = false;
    }
    
    AppMgr.switchToHome();
}

void WiFiConfigApp::btn_connect_cb(lv_event_t* e) {
    WiFiConfigApp* app = (WiFiConfigApp*)lv_event_get_user_data(e);
    
    if (app->textareaPassword) {
        const char* pass = lv_textarea_get_text(app->textareaPassword);
        strncpy(app->_password, pass, WIFI_MAX_PASS_LEN - 1);
        app->_password[WIFI_MAX_PASS_LEN - 1] = '\0';
    }
    
    app->hidePasswordDialog();
    app->startConnect(app->_selectedSSID, app->_password);
}

void WiFiConfigApp::btn_cancel_cb(lv_event_t* e) {
    WiFiConfigApp* app = (WiFiConfigApp*)lv_event_get_user_data(e);
    app->hidePasswordDialog();
}

void WiFiConfigApp::list_select_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    WiFiConfigApp* app = (WiFiConfigApp*)lv_event_get_user_data(e);
    
    for (int i = 0; i < app->_scanCount; i++) {
        if (lv_obj_get_child(app->listNetworks, i) == btn) {
            app->_selectedIndex = i;
            strncpy(app->_selectedSSID, app->_scanResults[i].ssid, WIFI_MAX_SSID_LEN - 1);
            app->_selectedSSID[WIFI_MAX_SSID_LEN - 1] = '\0';
            
            if (app->_scanResults[i].isOpen) {
                app->startConnect(app->_selectedSSID, "");
            } else {
                app->showPasswordDialog(app->_selectedSSID);
            }
            break;
        }
    }
}

void WiFiConfigApp::keyboard_event_cb(lv_event_t* e) {
    // 移除默认事件处理，避免重复输入
}

void WiFiConfigApp::startScan() {
    if (_mode != WIFI_MODE_IDLE) return;
    
    updateStatus("Scanning...");
    clearNetworkList();
    _mode = WIFI_MODE_SCANNING;
    
    // 强制重置WiFi模块
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(500);
    
    Serial.println("[WiFi] Starting scan...");
    Serial.printf("[WiFi] WiFi mode: %s\n", WiFi.getMode() == WIFI_MODE_STA ? "STA" : "AP");
    Serial.printf("[WiFi] Free heap: %u\n", ESP.getFreeHeap());
    
    // 使用异步扫描
    int n = WiFi.scanNetworks(false, true, false, WIFI_SCAN_TIMEOUT_MS);
    
    // 等待扫描完成
    unsigned long start = millis();
    while (WiFi.scanComplete() == WIFI_SCAN_RUNNING && millis() - start < WIFI_SCAN_TIMEOUT_MS) {
        delay(100);
    }
    
    n = WiFi.scanComplete();
    
    Serial.printf("[WiFi] Scan result: %d\n", n);
    
    if (n > 0) {
        _scanCount = min(n, WIFI_MAX_SCAN_RESULTS);
        
        for (int i = 0; i < _scanCount; i++) {
            Serial.printf("[WiFi] %d: %s (%d dBm)\n", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
            strncpy(_scanResults[i].ssid, WiFi.SSID(i).c_str(), WIFI_MAX_SSID_LEN - 1);
            _scanResults[i].ssid[WIFI_MAX_SSID_LEN - 1] = '\0';
            _scanResults[i].rssi = WiFi.RSSI(i);
            _scanResults[i].encryption = WiFi.encryptionType(i);
            _scanResults[i].isOpen = (_scanResults[i].encryption == WIFI_AUTH_OPEN);
        }
        
        // Sort by RSSI
        for (int i = 0; i < _scanCount - 1; i++) {
            for (int j = i + 1; j < _scanCount; j++) {
                if (_scanResults[j].rssi > _scanResults[i].rssi) {
                    wifi_scan_result_t swap = _scanResults[i];
                    _scanResults[i] = _scanResults[j];
                    _scanResults[j] = swap;
                }
            }
        }
        
        populateNetworkList();
        updateStatus("Found " + String(_scanCount) + " networks");
    } else {
        Serial.printf("[WiFi] Scan failed with code: %d\n", n);
        updateStatus("No networks found!");
    }
    
    _mode = WIFI_MODE_IDLE;
    WiFi.scanDelete();
}

void WiFiConfigApp::handleScanComplete() {
    int16_t result = WiFi.scanComplete();
    
    if (result == WIFI_SCAN_RUNNING) {
        return;
    }
    
    if (result < 0) {
        updateStatus("Scan failed!");
        _mode = WIFI_MODE_IDLE;
        return;
    }

    // Temporary array to hold unique results with strongest RSSI
    wifi_scan_result_t tempResults[WIFI_MAX_SCAN_RESULTS];
    int tempCount = 0;

    for (int i = 0; i < result; i++) {
        String currentSSID = WiFi.SSID(i);
        bool found = false;

        // Check if SSID already exists in temporary results
        for (int j = 0; j < tempCount; j++) {
            if (strcmp(tempResults[j].ssid, currentSSID.c_str()) == 0) {
                found = true;
                // Update entry if current has stronger signal
                if (WiFi.RSSI(i) > tempResults[j].rssi) {
                    tempResults[j].rssi = WiFi.RSSI(i);
                    tempResults[j].encryption = WiFi.encryptionType(i);
                    tempResults[j].isOpen = (tempResults[j].encryption == WIFI_AUTH_OPEN);
                }
                break;
            }
        }

        // Add new unique SSID to temporary results
        if (!found && tempCount < WIFI_MAX_SCAN_RESULTS) {
            strncpy(tempResults[tempCount].ssid, currentSSID.c_str(), WIFI_MAX_SSID_LEN -1);
            tempResults[tempCount].ssid[WIFI_MAX_SSID_LEN -1] = '\0';
            tempResults[tempCount].rssi = WiFi.RSSI(i);
            tempResults[tempCount].encryption = WiFi.encryptionType(i);
            tempResults[tempCount].isOpen = (tempResults[tempCount].encryption == WIFI_AUTH_OPEN);
            tempCount++;
        }
    }

    // Sort temporary results by RSSI descending (strongest first)
    for (int i = 0; i < tempCount -1; i++) {
        for (int j = i+1; j < tempCount; j++) {
            if (tempResults[j].rssi > tempResults[i].rssi) {
                wifi_scan_result_t swap = tempResults[i];
                tempResults[i] = tempResults[j];
                tempResults[j] = swap;
            }
        }
    }

    // Copy sorted unique results to main scan results array
    _scanCount = min(tempCount, WIFI_MAX_SCAN_RESULTS);
    for (int i =0; i < _scanCount; i++) {
        memcpy(&_scanResults[i], &tempResults[i], sizeof(wifi_scan_result_t));
    }
    
    populateNetworkList();
    updateStatus("Found " + String(_scanCount) + " unique networks");
    _mode = WIFI_MODE_IDLE;
    
    WiFi.scanDelete();
}

void WiFiConfigApp::clearNetworkList() {
    lv_obj_clean(listNetworks);
}

void WiFiConfigApp::populateNetworkList() {
    clearNetworkList();
    
    for (int i = 0; i < _scanCount; i++) {
        const char* icon = _scanResults[i].isOpen ? LV_SYMBOL_WIFI : "*";
        lv_obj_t* btn = lv_list_add_btn(listNetworks, icon, _scanResults[i].ssid);
        
        lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
        lv_obj_set_style_text_color(btn, lv_color_white(), 0);
        lv_obj_add_event_cb(btn, list_select_cb, LV_EVENT_CLICKED, this);
        
        lv_obj_t* label = lv_obj_get_child(btn, 0);
        if (label) {
            lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        }
    }
}

void WiFiConfigApp::showPasswordDialog(const char* ssid) {
    lv_obj_t* scr = getScreen();
    
    lv_obj_t* overlay = lv_obj_create(scr);
    lv_obj_set_size(overlay, BSP_DISPLAY_WIDTH - 20, 190);
    lv_obj_align(overlay, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(overlay, lv_color_make(0x30, 0x30, 0x50), 0);
    lv_obj_set_style_border_width(overlay, 2, 0);
    lv_obj_set_style_border_color(overlay, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_move_foreground(overlay);
    
    lv_obj_t* labelSSID = lv_label_create(overlay);
    lv_label_set_text_fmt(labelSSID, "Connect to: %s", ssid);
    lv_obj_set_style_text_color(labelSSID, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelSSID, &lv_font_montserrat_12, 0);
    lv_obj_align(labelSSID, LV_ALIGN_TOP_MID, 0, 5);
    
    textareaPassword = lv_textarea_create(overlay);
    lv_obj_set_size(textareaPassword, BSP_DISPLAY_WIDTH - 60, 35);
    lv_obj_align(textareaPassword, LV_ALIGN_TOP_MID, 0, 30);
    lv_textarea_set_placeholder_text(textareaPassword, "Enter password...");
    lv_textarea_set_password_mode(textareaPassword, false);
    lv_textarea_set_one_line(textareaPassword, true);
    lv_textarea_set_max_length(textareaPassword, WIFI_MAX_PASS_LEN);
    
    keyboard = lv_keyboard_create(overlay);
    lv_obj_set_size(keyboard, BSP_DISPLAY_WIDTH - 30, 70);
    lv_obj_align(keyboard, LV_ALIGN_TOP_MID, 0, 70);
    lv_keyboard_set_textarea(keyboard, textareaPassword);
    lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
    lv_obj_add_event_cb(keyboard, keyboard_event_cb, LV_EVENT_ALL, this);
    
    btnConnect = lv_btn_create(overlay);
    lv_obj_set_size(btnConnect, 80, 25);
    lv_obj_align(btnConnect, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_event_cb(btnConnect, btn_connect_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnConnect, lv_color_make(0x00, 0x80, 0x00), 0);
    lv_obj_t* labelConn = lv_label_create(btnConnect);
    lv_label_set_text(labelConn, LV_SYMBOL_OK " Connect");
    lv_obj_set_style_text_font(labelConn, &lv_font_montserrat_10, 0);
    lv_obj_center(labelConn);
    
    btnCancel = lv_btn_create(overlay);
    lv_obj_set_size(btnCancel, 80, 25);
    lv_obj_align(btnCancel, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_add_event_cb(btnCancel, btn_cancel_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnCancel, lv_color_make(0x80, 0x00, 0x00), 0);
    lv_obj_t* labelCancel = lv_label_create(btnCancel);
    lv_label_set_text(labelCancel, LV_SYMBOL_CLOSE " Cancel");
    lv_obj_set_style_text_font(labelCancel, &lv_font_montserrat_10, 0);
    lv_obj_center(labelCancel);
    
    lv_obj_set_user_data(overlay, (void*)0xDEADBEEF);
}

void WiFiConfigApp::hidePasswordDialog() {
    lv_obj_t* scr = getScreen();
    uint32_t childCount = lv_obj_get_child_cnt(scr);
    
    for (uint32_t i = 0; i < childCount; i++) {
        lv_obj_t* child = lv_obj_get_child(scr, i);
        if (lv_obj_get_user_data(child) == (void*)0xDEADBEEF) {
            lv_obj_del(child);
            break;
        }
    }
    
    keyboard = nullptr;
    textareaPassword = nullptr;
    btnConnect = nullptr;
    btnCancel = nullptr;
}

void WiFiConfigApp::startConnect(const char* ssid, const char* password) {
    updateStatus("Connecting...");
    _mode = WIFI_MODE_CONNECTING;
    _connecting = true;
    _connectAttempts = 0;
    _lastConnectStartMs = millis();
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}

void WiFiConfigApp::startAPMode() {
    updateStatus("Starting AP mode...");
    
    hideMainUI();
    
    snprintf(_apSSID, sizeof(_apSSID), "%s%08X", WIFI_AP_SSID_PREFIX, (uint32_t)ESP.getEfuseMac());
    strcpy(_apPassword, "12345678");
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(_apSSID, _apPassword);
    
    _apModeActive = true;
    _mode = WIFI_MODE_AP_CONFIG;
    
    showQRUI();
    
    char qrData[80];
    snprintf(qrData, sizeof(qrData), "WIFI:T:WPA;S:%s;P:%s;;", _apSSID, _apPassword);
    drawQRCode(qrData);
    
    updateStatus("AP Mode Active");
}

void WiFiConfigApp::stopAPMode() {
    if (_apModeActive) {
        WiFi.softAPdisconnect(true);
        _apModeActive = false;
        _mode = WIFI_MODE_IDLE;
        
        hideQRUI();
        showMainUI();
        
        updateStatus("AP Mode Stopped");
    }
}

void WiFiConfigApp::drawQRCode(const char* data) {
    if (!qrCanvas) return;
    
    uint8_t qrVersion = 3;
    uint8_t qrSize = qrVersion * 4 + 17;
    
    QRCode qrcode;
    uint8_t qrData[qrcode_getBufferSize(qrVersion)];
    
    qrcode_initText(&qrcode, qrData, qrVersion, ECC_LOW, data);
    
    lv_obj_set_size(qrCanvas, qrSize * 3 + 4, qrSize * 3 + 4);
    
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_white();
    
    lv_canvas_draw_rect(qrCanvas, 0, 0, qrSize * 3 + 4, qrSize * 3 + 4, &rect_dsc);
    
    rect_dsc.bg_color = lv_color_black();
    
    for (uint8_t y = 0; y < qrSize; y++) {
        for (uint8_t x = 0; x < qrSize; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                lv_canvas_draw_rect(qrCanvas, x * 3 + 2, y * 3 + 2, 3, 3, &rect_dsc);
            }
        }
    }
}

void WiFiConfigApp::showMainUI() {
    if (btnScan) lv_obj_clear_flag(btnScan, LV_OBJ_FLAG_HIDDEN);
    if (btnAPMode) lv_obj_clear_flag(btnAPMode, LV_OBJ_FLAG_HIDDEN);
    if (listNetworks) lv_obj_clear_flag(listNetworks, LV_OBJ_FLAG_HIDDEN);
}

void WiFiConfigApp::hideMainUI() {
    if (btnScan) lv_obj_add_flag(btnScan, LV_OBJ_FLAG_HIDDEN);
    if (btnAPMode) lv_obj_add_flag(btnAPMode, LV_OBJ_FLAG_HIDDEN);
    if (listNetworks) lv_obj_add_flag(listNetworks, LV_OBJ_FLAG_HIDDEN);
}

void WiFiConfigApp::showQRUI() {
    lv_obj_t* scr = getScreen();
    
    qrCanvas = lv_canvas_create(scr);
    lv_obj_align(qrCanvas, LV_ALIGN_CENTER, 0, -20);
    
    lv_color_t* canvasBuf = (lv_color_t*)malloc(120 * 120 * sizeof(lv_color_t));
    if (canvasBuf) {
        lv_canvas_set_buffer(qrCanvas, canvasBuf, 120, 120, LV_IMG_CF_TRUE_COLOR);
        lv_obj_set_user_data(qrCanvas, canvasBuf);
    }
    
    labelQRHint = lv_label_create(scr);
    lv_label_set_text(labelQRHint, "Scan QR to connect");
    lv_obj_set_style_text_color(labelQRHint, lv_color_make(0x00, 0xFF, 0xFF), 0);
    lv_obj_set_style_text_font(labelQRHint, &lv_font_montserrat_12, 0);
    lv_obj_align(labelQRHint, LV_ALIGN_BOTTOM_MID, 0, -50);
    
    labelAPInfo = lv_label_create(scr);
    lv_label_set_text_fmt(labelAPInfo, 
        "SSID: %s\nPass: %s\nIP: 192.168.4.1",
        _apSSID, _apPassword);
    lv_obj_set_style_text_color(labelAPInfo, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelAPInfo, &lv_font_montserrat_10, 0);
    lv_obj_align(labelAPInfo, LV_ALIGN_BOTTOM_MID, 0, -15);
    
    if (btnBack) {
        lv_obj_clear_flag(btnBack, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(btnBack, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    }
}

void WiFiConfigApp::hideQRUI() {
    if (qrCanvas) {
        lv_color_t* buf = (lv_color_t*)lv_obj_get_user_data(qrCanvas);
        if (buf) {
            free(buf);
        }
        lv_obj_del(qrCanvas);
        qrCanvas = nullptr;
    }
    if (labelQRHint) {
        lv_obj_del(labelQRHint);
        labelQRHint = nullptr;
    }
    if (labelAPInfo) {
        lv_obj_del(labelAPInfo);
        labelAPInfo = nullptr;
    }
}

void WiFiConfigApp::updateStatus(const char* text) {
    if (labelStatus) {
        lv_label_set_text(labelStatus, text);
    }
}

void WiFiConfigApp::updateStatus(const String& text) {
    updateStatus(text.c_str());
}

const char* WiFiConfigApp::getEncryptionType(wifi_auth_mode_t enc) {
    switch (enc) {
        case WIFI_AUTH_OPEN: return "Open";
        case WIFI_AUTH_WEP: return "WEP";
        case WIFI_AUTH_WPA_PSK: return "WPA";
        case WIFI_AUTH_WPA2_PSK: return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
        case WIFI_AUTH_WPA3_PSK: return "WPA3";
        default: return "Unknown";
    }
}

void WiFiConfigApp::saveWiFiConfig(const char* ssid, const char* password) {
    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.end();
    
    if (Storage.isSDReady()) {
        saveWiFiConfigToSD(ssid, password);
    }
}

bool WiFiConfigApp::saveWiFiConfigToSD(const char* ssid, const char* password) {
    if (!Storage.isSDReady()) {
        Serial.println("[WiFi] SD not ready, cannot save config");
        return false;
    }
    
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["password"] = password;
    
    File file = SD.open(WIFI_CONFIG_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("[WiFi] Failed to open config file for writing");
        return false;
    }
    
    serializeJson(doc, file);
    file.close();
    
    Serial.printf("[WiFi] Saved config to SD: %s\n", ssid);
    
    strncpy(_lastSSID, ssid, WIFI_MAX_SSID_LEN - 1);
    _lastSSID[WIFI_MAX_SSID_LEN - 1] = '\0';
    strncpy(_lastPassword, password, WIFI_MAX_PASS_LEN - 1);
    _lastPassword[WIFI_MAX_PASS_LEN - 1] = '\0';
    _lastConfigValid = true;
    
    updateLastConnectButton();
    
    return true;
}

bool WiFiConfigApp::loadWiFiConfigFromSD() {
    if (!Storage.isSDReady()) {
        Serial.println("[WiFi] SD not ready, cannot load config");
        _lastConfigValid = false;
        return false;
    }
    
    if (!SD.exists(WIFI_CONFIG_FILE)) {
        Serial.println("[WiFi] No config file found on SD");
        _lastConfigValid = false;
        return false;
    }
    
    File file = SD.open(WIFI_CONFIG_FILE, FILE_READ);
    if (!file) {
        Serial.println("[WiFi] Failed to open config file for reading");
        _lastConfigValid = false;
        return false;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("[WiFi] JSON parse error: %s\n", error.c_str());
        _lastConfigValid = false;
        return false;
    }
    
    const char* ssid = doc["ssid"];
    const char* pass = doc["password"];
    
    if (ssid && strlen(ssid) > 0) {
        strncpy(_lastSSID, ssid, WIFI_MAX_SSID_LEN - 1);
        _lastSSID[WIFI_MAX_SSID_LEN - 1] = '\0';
        if (pass) {
            strncpy(_lastPassword, pass, WIFI_MAX_PASS_LEN - 1);
            _lastPassword[WIFI_MAX_PASS_LEN - 1] = '\0';
        } else {
            _lastPassword[0] = '\0';
        }
        _lastConfigValid = true;
        Serial.printf("[WiFi] Loaded config from SD: %s\n", _lastSSID);
        return true;
    }
    
    _lastConfigValid = false;
    return false;
}

bool WiFiConfigApp::hasLastConfig() {
    return _lastConfigValid && strlen(_lastSSID) > 0;
}

void WiFiConfigApp::startLastConnect() {
    if (!hasLastConfig()) {
        updateStatus("No saved config!");
        return;
    }
    
    updateStatus("Connecting to last...");
    _mode = WIFI_MODE_CONNECTING;
    _connecting = true;
    _connectAttempts = 0;
    _lastConnectStartMs = millis();
    
    strncpy(_selectedSSID, _lastSSID, WIFI_MAX_SSID_LEN - 1);
    _selectedSSID[WIFI_MAX_SSID_LEN - 1] = '\0';
    strncpy(_password, _lastPassword, WIFI_MAX_PASS_LEN - 1);
    _password[WIFI_MAX_PASS_LEN - 1] = '\0';
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(_lastSSID, _lastPassword);
    
    Serial.printf("[WiFi] Connecting to last: %s\n", _lastSSID);
}

void WiFiConfigApp::updateLastConnectButton() {
    if (!btnLastConnect) return;
    
    if (hasLastConfig()) {
        lv_obj_clear_state(btnLastConnect, LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(btnLastConnect, lv_color_make(0x00, 0x80, 0x00), 0);
    } else {
        lv_obj_add_state(btnLastConnect, LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(btnLastConnect, lv_color_make(0x40, 0x40, 0x40), 0);
    }
}

bool WiFiConfigApp::loadWiFiConfig() {
    Preferences prefs;
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    prefs.end();
    
    if (ssid.length() > 0) {
        strncpy(_selectedSSID, ssid.c_str(), WIFI_MAX_SSID_LEN - 1);
        strncpy(_password, pass.c_str(), WIFI_MAX_PASS_LEN - 1);
        return true;
    }
    return false;
}

void WiFiConfigApp::onUpdate() {
    uint32_t now = millis();
    
    if (now - _lastUpdateMs < 100) {
        return;
    }
    _lastUpdateMs = now;
    
    if (_mode == WIFI_MODE_CONNECTING && _connecting) {
        wl_status_t status = WiFi.status();
        
        if (status == WL_CONNECTED) {
            _mode = WIFI_MODE_CONNECTED;
            _connecting = false;
            
            updateStatus("Connected! IP: " + WiFi.localIP().toString());
            saveWiFiConfig(_selectedSSID, _password);
            
            bsp_rgb_led_set(0, 255, 0);
        }
        else if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
            _mode = WIFI_MODE_FAILED;
            _connecting = false;
            updateStatus("Connection failed!");
            
            bsp_rgb_led_set(255, 0, 0);
        }
        else {
            _connectAttempts++;
            uint32_t elapsed = now - _lastConnectStartMs;
            if (elapsed > WIFI_LAST_CONNECT_TIMEOUT_MS || _connectAttempts > 100) {
                _mode = WIFI_MODE_FAILED;
                _connecting = false;
                WiFi.disconnect();
                updateStatus("Connection timeout!");
                
                bsp_rgb_led_set(255, 0, 0);
            }
        }
    }
    else if (_mode == WIFI_MODE_AP_CONFIG && _apModeActive) {
        int stations = WiFi.softAPgetStationNum();
        if (stations > 0) {
            updateStatus("Client connected!");
        }
    }
}

app_info_t WiFiConfigApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "WiFiConfig", APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, LV_SYMBOL_WIFI);
    info.type = APP_TYPE_SYSTEM;
    info.enabled = true;
    return info;
}

BaseApp* createWiFiConfigApp() {
    return new WiFiConfigApp();
}
