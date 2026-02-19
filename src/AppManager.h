#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <Arduino.h>
#include <lvgl.h>
#include <vector>

#define APP_MAX_APPS            8
#define APP_NAME_MAX_LEN        24
#define APP_STATE_FILE          "/app_state.json"

typedef enum {
    APP_STATE_STOPPED = 0,
    APP_STATE_PAUSED = 1,
    APP_STATE_ACTIVE = 2
} app_state_t;

typedef enum {
    APP_TYPE_SYSTEM = 0,
    APP_TYPE_USER = 1
} app_type_t;

typedef struct {
    char name[APP_NAME_MAX_LEN];
    char icon[32];
    app_type_t type;
    bool enabled;
} app_info_t;

class BaseApp {
protected:
    char _name[APP_NAME_MAX_LEN];
    app_state_t _state;
    lv_obj_t* _screen;
    bool _initialized;
    
public:
    BaseApp(const char* name);
    virtual ~BaseApp();
    
    const char* getName() const { return _name; }
    app_state_t getState() const { return _state; }
    lv_obj_t* getScreen() const { return _screen; }
    bool isInitialized() const { return _initialized; }
    
    virtual bool onCreate();
    virtual void onDestroy();
    
    virtual bool onResume();
    virtual void onPause();
    
    virtual void onUpdate();
    
    virtual app_info_t getInfo() const;
    
    void setState(app_state_t state) { _state = state; }
    
protected:
    virtual bool createUI() { return true; }
    virtual void destroyUI();
    virtual void saveState() {}
    virtual bool loadState() { return false; }
};

typedef BaseApp* (*app_factory_t)(void);

typedef struct {
    char name[APP_NAME_MAX_LEN];
    app_factory_t factory;
    app_info_t info;
    bool registered;
} app_entry_t;

class AppManager {
private:
    app_entry_t _apps[APP_MAX_APPS];
    int _appCount;
    
    BaseApp* _activeApp;
    BaseApp* _pausedApp;
    
    lv_obj_t* _homeScreen;
    lv_obj_t* _appContainer;
    
    bool _switching;
    uint32_t _switchStartTime;
    
    void cleanupPausedApp();
    bool preloadAppResources(const char* appName);
    
public:
    AppManager();
    ~AppManager();
    
    bool begin();
    void update();
    
    bool registerApp(const char* name, app_factory_t factory, const app_info_t* info = nullptr);
    
    bool switchToApp(const char* name);
    bool switchToHome();
    bool closeCurrentApp();
    
    BaseApp* getActiveApp() const { return _activeApp; }
    BaseApp* getPausedApp() const { return _pausedApp; }
    
    int getAppCount() const { return _appCount; }
    app_info_t getAppInfo(int index) const;
    const char* getAppName(int index) const;
    BaseApp* findApp(const char* name);
    
    bool isAppRegistered(const char* name);
    bool isAppEnabled(const char* name);
    
    void setAppEnabled(const char* name, bool enabled);
    
    void createHomeScreen();
    lv_obj_t* getHomeScreen() const { return _homeScreen; }
    
    void printStatus();
};

extern AppManager AppMgr;

#endif
