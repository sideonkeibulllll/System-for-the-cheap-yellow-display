#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "ConfigManager.h"
#include "BSP.h"
#include "Storage.h"
#include "Performance.h"
#include "PowerManager.h"
#include "AppManager.h"
#include "SettingsApp.h"
#include "DemoApp.h"
#include "WiFiConfigApp.h"
#include "FileExplorerApp.h"
#include "ChatApp.h"
#include "DictionaryApp.h"
#include "WordCardApp.h"
#include "GlobalUI.h"
#include "LvZhFont.h"
#include "XFontAdapter.h"

static TaskHandle_t appTaskHandle = nullptr;

static lv_obj_t* labelHomeStatus;
static lv_obj_t* labelHomePerf;
static lv_obj_t* labelTime;
static lv_obj_t* labelDate;
static lv_timer_t* homeUpdateTimer = nullptr;
static lv_timer_t* timeUpdateTimer = nullptr;

static const char* weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

static void home_update_cb(lv_timer_t* timer) {
    if (labelHomePerf && lv_obj_is_valid(labelHomePerf)) {
        perf_stats_t stats = Perf.getStats();
        lv_label_set_text_fmt(labelHomePerf, 
            "FPS: %u | Heap: %u KB | LVGL: %u KB",
            stats.fps,
            stats.freeHeap / 1024,
            stats.lvglMemUsed / 1024
        );
    }
}

static void time_update_cb(lv_timer_t* timer) {
    if (labelTime && lv_obj_is_valid(labelTime)) {
        struct tm timeinfo;
        time_t now;
        time(&now);
        localtime_r(&now, &timeinfo);
        lv_label_set_text_fmt(labelTime, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        
        if (labelDate && lv_obj_is_valid(labelDate)) {
            lv_label_set_text_fmt(labelDate, "%d/%d %s", 
                timeinfo.tm_mon + 1, 
                timeinfo.tm_mday, 
                weekdays[timeinfo.tm_wday]);
        }
    }
}

static void app_btn_cb(lv_event_t* e) {
    const char* appName = (const char*)lv_event_get_user_data(e);
    Serial.printf("[Home] Launching app: %s\n", appName);
    AppMgr.switchToApp(appName);
    bsp_rgb_led_set(rand() % 256, rand() % 256, rand() % 256);
}

static void createHomeUI() {
    lv_obj_t* home = AppMgr.getHomeScreen();
    if (!home) return;
    
    lv_obj_t* container = lv_obj_create(home);
    lv_obj_set_size(container, BSP_DISPLAY_WIDTH - 20, 120);
    lv_obj_set_style_bg_color(container, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_align(container, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(container, 10, 0);
    lv_obj_set_style_pad_column(container, 10, 0);
    
    int appCount = AppMgr.getAppCount();
    for (int i = 0; i < appCount; i++) {
        app_info_t info = AppMgr.getAppInfo(i);
        if (!info.enabled) continue;
        
        const char* appName = AppMgr.getAppName(i);
        
        lv_obj_t* appBtn = lv_btn_create(container);
        lv_obj_set_size(appBtn, 90, 50);
        lv_obj_add_event_cb(appBtn, app_btn_cb, LV_EVENT_CLICKED, (void*)appName);
        
        if (info.type == APP_TYPE_SYSTEM) {
            lv_obj_set_style_bg_color(appBtn, lv_color_make(0x00, 0x60, 0x80), 0);
        } else {
            lv_obj_set_style_bg_color(appBtn, lv_color_make(0x60, 0x40, 0x80), 0);
        }
        
        lv_obj_t* btnLabel = lv_label_create(appBtn);
        lv_label_set_text(btnLabel, info.icon);
        lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_20, 0);
        lv_obj_center(btnLabel);
        
        lv_obj_t* nameLabel = lv_label_create(appBtn);
        lv_label_set_text(nameLabel, info.name);
        lv_obj_set_style_text_font(nameLabel, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(nameLabel, lv_color_white(), 0);
        lv_obj_align(nameLabel, LV_ALIGN_BOTTOM_MID, 0, -2);
    }
    
    lv_obj_t* timeContainer = lv_obj_create(home);
    lv_obj_set_size(timeContainer, BSP_DISPLAY_WIDTH - 20, 75);
    lv_obj_set_style_bg_color(timeContainer, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(timeContainer, 1, 0);
    lv_obj_set_style_border_color(timeContainer, lv_color_black(), 0);
    lv_obj_set_style_border_side(timeContainer, LV_BORDER_SIDE_FULL, 0);
    lv_obj_set_style_border_post(timeContainer, true, 0);
    lv_obj_set_style_radius(timeContainer, 0, 0);
    lv_obj_align(timeContainer, LV_ALIGN_TOP_MID, 0, 160);
    
    labelTime = lv_label_create(timeContainer);
    lv_label_set_text(labelTime, "--:--:--");
    lv_obj_set_style_text_color(labelTime, lv_color_black(), 0);
    lv_obj_set_style_text_font(labelTime, &lv_font_montserrat_28, 0);
    lv_obj_align(labelTime, LV_ALIGN_TOP_MID, 0, 5);
    
    labelDate = lv_label_create(timeContainer);
    lv_label_set_text(labelDate, "-/- ------");
    lv_obj_set_style_text_color(labelDate, lv_color_make(0x40, 0x40, 0x40), 0);
    lv_obj_set_style_text_font(labelDate, &lv_font_montserrat_14, 0);
    lv_obj_align(labelDate, LV_ALIGN_BOTTOM_MID, 0, -5);
    
    timeUpdateTimer = lv_timer_create(time_update_cb, 1000, NULL);
    
    homeUpdateTimer = lv_timer_create(home_update_cb, 500, NULL);
}

static void power_state_callback(power_state_t newState) {
    const char* stateStr = newState == POWER_STATE_ACTIVE ? "Active" :
                           newState == POWER_STATE_IDLE ? "Idle" : "Sleep";
    Serial.printf("[App] Power state: %s\n", stateStr);
}

static void backlight_mode_callback(backlight_mode_t newMode) {
    const char* modeStr = newMode == BACKLIGHT_MODE_MANUAL ? "Manual" :
                          newMode == BACKLIGHT_MODE_AUTO ? "Auto" : "Off";
    Serial.printf("[App] Backlight mode: %s\n", modeStr);
}

static void appTaskEntry(void* arg) {
    Serial.println("[App] Application task running on Core 0");
    
    while (true) {
        Power.update();
        AppMgr.update();
        XFontAdapter::instance.update();
        
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  ESP32-2432S028R (CYD) v3 - Stage 7");
    Serial.println("  App Manager & Multi-App System");
    Serial.println("========================================\n");
    
    Serial.printf("Initial CPU Freq: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("Running on Core: %d\n", xPortGetCoreID());
    
    Config.begin();
    lv_init();
    
    Perf.begin();
    
    bsp_init();
    
    Power.begin();
    Power.setStateCallback(power_state_callback);
    Power.setBacklightModeCallback(backlight_mode_callback);
    
    LvZhFontMgr.begin();
    
    AppMgr.begin();
    
    app_info_t settingsInfo;
    strncpy(settingsInfo.name, "Settings", APP_NAME_MAX_LEN - 1);
    strcpy(settingsInfo.icon, LV_SYMBOL_SETTINGS);
    settingsInfo.type = APP_TYPE_SYSTEM;
    settingsInfo.enabled = true;
    AppMgr.registerApp("Settings", createSettingsApp, &settingsInfo);
    
    app_info_t demoInfo;
    strncpy(demoInfo.name, "Demo", APP_NAME_MAX_LEN - 1);
    strcpy(demoInfo.icon, LV_SYMBOL_PLAY);
    demoInfo.type = APP_TYPE_USER;
    demoInfo.enabled = true;
    AppMgr.registerApp("Demo", createDemoApp, &demoInfo);
    
    app_info_t wifiInfo;
    strncpy(wifiInfo.name, "WiFi", APP_NAME_MAX_LEN - 1);
    strcpy(wifiInfo.icon, LV_SYMBOL_WIFI);
    wifiInfo.type = APP_TYPE_SYSTEM;
    wifiInfo.enabled = true;
    AppMgr.registerApp("WiFiConfig", createWiFiConfigApp, &wifiInfo);
    
    app_info_t fileInfo;
    strncpy(fileInfo.name, "Files", APP_NAME_MAX_LEN - 1);
    strcpy(fileInfo.icon, LV_SYMBOL_DIRECTORY);
    fileInfo.type = APP_TYPE_SYSTEM;
    fileInfo.enabled = true;
    AppMgr.registerApp("FileExplorer", createFileExplorerApp, &fileInfo);
    
    app_info_t chatInfo;
    strncpy(chatInfo.name, "Chat", APP_NAME_MAX_LEN - 1);
    strcpy(chatInfo.icon, LV_SYMBOL_EDIT);
    chatInfo.type = APP_TYPE_USER;
    chatInfo.enabled = true;
    AppMgr.registerApp("Chat", createChatApp, &chatInfo);
    
    app_info_t dictInfo;
    strncpy(dictInfo.name, "Dictionary", APP_NAME_MAX_LEN - 1);
    strcpy(dictInfo.icon, LV_SYMBOL_FILE);
    dictInfo.type = APP_TYPE_USER;
    dictInfo.enabled = true;
    AppMgr.registerApp("Dictionary", createDictionaryApp, &dictInfo);
    
    app_info_t wordCardInfo;
    strncpy(wordCardInfo.name, "WordCard", APP_NAME_MAX_LEN - 1);
    strcpy(wordCardInfo.icon, LV_SYMBOL_EDIT);
    wordCardInfo.type = APP_TYPE_USER;
    wordCardInfo.enabled = true;
    AppMgr.registerApp("WordCard", createWordCardApp, &wordCardInfo);
    
    bsp_rgb_led_set(255, 0, 0);
    delay(200);
    bsp_rgb_led_set(0, 255, 0);
    delay(200);
    bsp_rgb_led_set(0, 0, 255);
    delay(200);
    bsp_rgb_led_off();
    
    bsp_print_status();
    Storage.printStatus();
    Power.printStatus();
    AppMgr.printStatus();
    
    createHomeUI();
    
    lv_scr_load(AppMgr.getHomeScreen());
    
    GlobalUI::getInstance().init();
    
    Perf.startLvglTask();
    
    xTaskCreatePinnedToCore(
        appTaskEntry,
        "AppTask",
        4096,
        NULL,
        3,
        &appTaskHandle,
        0
    );
    
    Serial.println("\n========================================");
    Serial.println("  Stage 7: App Manager READY");
    Serial.println("========================================");
    Serial.println("  Home Screen: App Launcher");
    Serial.println("  Settings: Brightness, Power config");
    Serial.println("  Demo: Counter app demo");
    Serial.println("  BOOT Button: Cycle backlight mode");
    Serial.println("  Touch: App navigation\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
