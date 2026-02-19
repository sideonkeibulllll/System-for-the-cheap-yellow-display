#include "AppManager.h"
#include "BSP.h"
#include "Storage.h"
#include "PowerManager.h"
#include "GlobalUI.h"
#include <lvgl.h>
#include <Arduino.h>
#include <cstring>

AppManager AppMgr;

BaseApp::BaseApp(const char* name) {
    strncpy(_name, name, APP_NAME_MAX_LEN - 1);
    _name[APP_NAME_MAX_LEN - 1] = '\0';
    _state = APP_STATE_STOPPED;
    _screen = nullptr;
    _initialized = false;
}

BaseApp::~BaseApp() {
    if (_initialized) {
        onDestroy();
    }
}

bool BaseApp::onCreate() {
    if (_initialized) {
        return true;
    }
    
    _screen = lv_obj_create(nullptr);
    if (!_screen) {
        Serial.printf("[App] Failed to create screen for %s\n", _name);
        return false;
    }
    
    lv_obj_set_style_bg_color(_screen, lv_color_black(), 0);
    lv_obj_set_size(_screen, BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT);
    
    if (!createUI()) {
        Serial.printf("[App] Failed to create UI for %s\n", _name);
        lv_obj_del(_screen);
        _screen = nullptr;
        return false;
    }
    
    _initialized = true;
    _state = APP_STATE_STOPPED;
    
    Serial.printf("[App] %s created successfully\n", _name);
    return true;
}

void BaseApp::onDestroy() {
    if (!_initialized) {
        return;
    }
    
    saveState();
    destroyUI();
    
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
    
    _initialized = false;
    _state = APP_STATE_STOPPED;
    
    Serial.printf("[App] %s destroyed\n", _name);
}

bool BaseApp::onResume() {
    if (!_initialized) {
        if (!onCreate()) {
            return false;
        }
    }
    
    loadState();
    _state = APP_STATE_ACTIVE;
    
    if (_screen) {
        lv_scr_load(_screen);
        GlobalUI::getInstance().init();
    }
    
    Serial.printf("[App] %s resumed\n", _name);
    return true;
}

void BaseApp::onPause() {
    if (_state != APP_STATE_ACTIVE) {
        return;
    }
    
    saveState();
    _state = APP_STATE_PAUSED;
    
    Serial.printf("[App] %s paused\n", _name);
}

void BaseApp::onUpdate() {
}

void BaseApp::destroyUI() {
}

app_info_t BaseApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, _name, APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, "app");
    info.type = APP_TYPE_USER;
    info.enabled = true;
    return info;
}

AppManager::AppManager() {
    _appCount = 0;
    _activeApp = nullptr;
    _pausedApp = nullptr;
    _homeScreen = nullptr;
    _appContainer = nullptr;
    _switching = false;
    _switchStartTime = 0;
    
    memset(_apps, 0, sizeof(_apps));
}

AppManager::~AppManager() {
    if (_activeApp) {
        _activeApp->onPause();
        _activeApp->onDestroy();
        delete _activeApp;
        _activeApp = nullptr;
    }
    
    if (_pausedApp) {
        _pausedApp->onDestroy();
        delete _pausedApp;
        _pausedApp = nullptr;
    }
    
    if (_homeScreen) {
        lv_obj_del(_homeScreen);
        _homeScreen = nullptr;
    }
}

bool AppManager::begin() {
    Serial.println("[AppMgr] Initializing App Manager...");
    
    createHomeScreen();
    
    Serial.printf("[AppMgr] Registered %d apps\n", _appCount);
    return true;
}

void AppManager::update() {
    if (_switching && millis() - _switchStartTime > 100) {
        _switching = false;
    }
    
    if (_activeApp && _activeApp->getState() == APP_STATE_ACTIVE) {
        _activeApp->onUpdate();
    }
}

bool AppManager::registerApp(const char* name, app_factory_t factory, const app_info_t* info) {
    if (_appCount >= APP_MAX_APPS) {
        Serial.println("[AppMgr] Maximum apps reached");
        return false;
    }
    
    if (isAppRegistered(name)) {
        Serial.printf("[AppMgr] App %s already registered\n", name);
        return false;
    }
    
    app_entry_t* entry = &_apps[_appCount];
    strncpy(entry->name, name, APP_NAME_MAX_LEN - 1);
    entry->name[APP_NAME_MAX_LEN - 1] = '\0';
    entry->factory = factory;
    entry->registered = true;
    
    if (info) {
        memcpy(&entry->info, info, sizeof(app_info_t));
    } else {
        strncpy(entry->info.name, name, APP_NAME_MAX_LEN - 1);
        entry->info.name[APP_NAME_MAX_LEN - 1] = '\0';
        strcpy(entry->info.icon, "app");
        entry->info.type = APP_TYPE_USER;
        entry->info.enabled = true;
    }
    
    _appCount++;
    Serial.printf("[AppMgr] Registered app: %s\n", name);
    return true;
}

bool AppManager::switchToApp(const char* name) {
    if (_switching) {
        Serial.println("[AppMgr] Switch in progress, ignoring request");
        return false;
    }
    
    for (int i = 0; i < _appCount; i++) {
        if (strcmp(_apps[i].name, name) == 0 && _apps[i].registered) {
            if (!_apps[i].info.enabled) {
                Serial.printf("[AppMgr] App %s is disabled\n", name);
                return false;
            }
            
            _switching = true;
            _switchStartTime = millis();
            
            if (_activeApp) {
                _activeApp->onPause();
                
                if (_pausedApp && _pausedApp != _activeApp) {
                    _pausedApp->onDestroy();
                    delete _pausedApp;
                }
                
                _pausedApp = _activeApp;
                _activeApp = nullptr;
            }
            
            BaseApp* newApp = _apps[i].factory();
            if (!newApp) {
                Serial.printf("[AppMgr] Failed to create app %s\n", name);
                return false;
            }
            
            if (!newApp->onResume()) {
                Serial.printf("[AppMgr] Failed to resume app %s\n", name);
                delete newApp;
                return false;
            }
            
            _activeApp = newApp;
            Power.resetIdleTimer();
            
            Serial.printf("[AppMgr] Switched to app: %s\n", name);
            return true;
        }
    }
    
    Serial.printf("[AppMgr] App %s not found\n", name);
    return false;
}

bool AppManager::switchToHome() {
    if (_switching) {
        return false;
    }
    
    _switching = true;
    _switchStartTime = millis();
    
    if (_homeScreen) {
        lv_scr_load(_homeScreen);
        GlobalUI::getInstance().init();
    }
    
    if (_activeApp) {
        _activeApp->onPause();
        _activeApp->onDestroy();
        delete _activeApp;
        _activeApp = nullptr;
    }
    
    cleanupPausedApp();
    
    Power.resetIdleTimer();
    Serial.println("[AppMgr] Returned to home screen");
    return true;
}

bool AppManager::closeCurrentApp() {
    return switchToHome();
}

void AppManager::cleanupPausedApp() {
    if (_pausedApp) {
        _pausedApp->onDestroy();
        delete _pausedApp;
        _pausedApp = nullptr;
    }
}

bool AppManager::preloadAppResources(const char* appName) {
    char manifestPath[64];
    snprintf(manifestPath, sizeof(manifestPath), "F:/%s_manifest.json", appName);
    
    return Storage.preloadResources(manifestPath);
}

app_info_t AppManager::getAppInfo(int index) const {
    if (index >= 0 && index < _appCount) {
        return _apps[index].info;
    }
    
    app_info_t empty;
    memset(&empty, 0, sizeof(empty));
    return empty;
}

const char* AppManager::getAppName(int index) const {
    if (index >= 0 && index < _appCount) {
        return _apps[index].name;
    }
    return nullptr;
}

BaseApp* AppManager::findApp(const char* name) {
    if (_activeApp && strcmp(_activeApp->getName(), name) == 0) {
        return _activeApp;
    }
    if (_pausedApp && strcmp(_pausedApp->getName(), name) == 0) {
        return _pausedApp;
    }
    return nullptr;
}

bool AppManager::isAppRegistered(const char* name) {
    for (int i = 0; i < _appCount; i++) {
        if (strcmp(_apps[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

bool AppManager::isAppEnabled(const char* name) {
    for (int i = 0; i < _appCount; i++) {
        if (strcmp(_apps[i].name, name) == 0) {
            return _apps[i].info.enabled;
        }
    }
    return false;
}

void AppManager::setAppEnabled(const char* name, bool enabled) {
    for (int i = 0; i < _appCount; i++) {
        if (strcmp(_apps[i].name, name) == 0) {
            _apps[i].info.enabled = enabled;
            Serial.printf("[AppMgr] App %s %s\n", name, enabled ? "enabled" : "disabled");
            return;
        }
    }
}

void AppManager::createHomeScreen() {
    _homeScreen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(_homeScreen, lv_color_black(), 0);
    lv_obj_set_size(_homeScreen, BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT);
    
    lv_obj_t* title = lv_label_create(_homeScreen);
    lv_label_set_text(title, "ESP32 App Launcher");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    lv_obj_t* hint = lv_label_create(_homeScreen);
    lv_label_set_text(hint, LV_SYMBOL_LEFT " Back  |  " LV_SYMBOL_POWER " Sleep");
    lv_obj_set_style_text_color(hint, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_12, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -5);
    
    _appContainer = lv_obj_create(_homeScreen);
    lv_obj_set_size(_appContainer, BSP_DISPLAY_WIDTH - 10, BSP_DISPLAY_HEIGHT - 50);
    lv_obj_set_style_bg_color(_appContainer, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(_appContainer, 0, 0);
    lv_obj_align(_appContainer, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_flex_flow(_appContainer, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(_appContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(_appContainer, 5, 0);
    lv_obj_set_style_pad_column(_appContainer, 5, 0);
    
    Serial.println("[AppMgr] Home screen created");
}

void AppManager::printStatus() {
    Serial.println("\n[AppMgr] ===== App Manager Status =====");
    Serial.printf("  Registered Apps: %d\n", _appCount);
    Serial.printf("  Active App: %s\n", _activeApp ? _activeApp->getName() : "None");
    Serial.printf("  Paused App: %s\n", _pausedApp ? _pausedApp->getName() : "None");
    
    for (int i = 0; i < _appCount; i++) {
        Serial.printf("  [%d] %s (%s)\n", i, _apps[i].name, 
                     _apps[i].info.enabled ? "enabled" : "disabled");
    }
    Serial.println("[AppMgr] ==============================");
}
