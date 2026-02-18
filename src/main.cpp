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

static lv_obj_t* labelStatus;
static lv_obj_t* labelPerf;
static lv_obj_t* labelPower;
static lv_obj_t* btnTest;
static int clickCount = 0;

static TaskHandle_t appTaskHandle = nullptr;

static void btn_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        clickCount++;
        lv_label_set_text_fmt(labelStatus, "Clicked %d times", clickCount);
        bsp_rgb_led_set(rand() % 256, rand() % 256, rand() % 256);
        Power.resetIdleTimer();
    }
}

static void perf_update_cb(lv_timer_t* timer) {
    static uint32_t lastTick = 0;
    uint32_t currentTick = millis();
    
    if (currentTick - lastTick >= 1000) {
        lastTick = currentTick;
        
        perf_stats_t stats = Perf.getStats();
        
        lv_label_set_text_fmt(labelPerf, 
            "FPS: %u | Heap: %u KB\n"
            "LVGL: %u KB used",
            stats.fps,
            stats.freeHeap / 1024,
            stats.lvglMemUsed / 1024
        );
    }
}

static void power_update_cb(lv_timer_t* timer) {
    power_status_t status = Power.getStatus();
    
    const char* stateStr = status.state == POWER_STATE_ACTIVE ? "Active" :
                           status.state == POWER_STATE_IDLE ? "Idle" : "Sleep";
    
    const char* blModeStr = status.backlightMode == BACKLIGHT_MODE_MANUAL ? "Manual" :
                            status.backlightMode == BACKLIGHT_MODE_AUTO ? "Auto" : "Off";
    
    lv_label_set_text_fmt(labelPower,
        "Power: %s | CPU: %dMHz\n"
        "BL: %s (%d) | LDR: %d\n"
        "Idle: %us | BOOT: %u",
        stateStr,
        getCpuFrequencyMhz(),
        blModeStr,
        status.backlightLevel,
        status.ldrValue,
        status.idleTimeMs / 1000,
        status.bootPressCount
    );
}

void createUI() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Stage 6: Power Manager");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, 
        "BOOT Button: Cycle BL Mode\n"
        "MANUAL -> AUTO -> OFF -> MANUAL\n"
        "Auto: LDR brightness adjust\n"
        "Sleep: 5min idle"
    );
    lv_obj_set_style_text_color(info, lv_color_white(), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_12, 0);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 5, 30);
    
    btnTest = lv_btn_create(scr);
    lv_obj_set_size(btnTest, 140, 40);
    lv_obj_align(btnTest, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btnTest, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(btnTest, lv_color_make(0x00, 0x80, 0xFF), 0);
    
    lv_obj_t* btnLabel = lv_label_create(btnTest);
    lv_label_set_text(btnLabel, "Touch Me!");
    lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_14, 0);
    lv_obj_center(btnLabel);
    
    labelStatus = lv_label_create(scr);
    lv_label_set_text(labelStatus, "Power management ready!");
    lv_obj_set_style_text_color(labelStatus, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_12, 0);
    lv_obj_align(labelStatus, LV_ALIGN_BOTTOM_MID, 0, -70);
    
    labelPower = lv_label_create(scr);
    lv_label_set_text(labelPower, "Power: -- | CPU: --MHz\nBL: -- | LDR: --");
    lv_obj_set_style_text_color(labelPower, lv_color_make(0x00, 0xFF, 0xFF), 0);
    lv_obj_set_style_text_font(labelPower, &lv_font_montserrat_12, 0);
    lv_obj_align(labelPower, LV_ALIGN_BOTTOM_MID, 0, -40);
    
    labelPerf = lv_label_create(scr);
    lv_label_set_text(labelPerf, "FPS: -- | Heap: -- KB");
    lv_obj_set_style_text_color(labelPerf, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelPerf, &lv_font_montserrat_12, 0);
    lv_obj_align(labelPerf, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    lv_timer_create(perf_update_cb, 100, NULL);
    lv_timer_create(power_update_cb, 500, NULL);
}

static void power_state_callback(power_state_t newState) {
    Serial.printf("[App] Power state changed: %d\n", newState);
}

static void backlight_mode_callback(backlight_mode_t newMode) {
    Serial.printf("[App] Backlight mode changed: %d\n", newMode);
}

static void appTaskEntry(void* arg) {
    Serial.println("[App] Application task running on Core 0");
    
    while (true) {
        Power.update();
        
        static uint32_t lastPrint = 0;
        uint32_t now = millis();
        
        if (now - lastPrint >= 10000) {
            lastPrint = now;
            Power.printStatus();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  ESP32-2432S028R (CYD) v3 - Stage 6");
    Serial.println("  Power Management System");
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
    
    createUI();
    
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
    Serial.println("  Stage 6: Power Management READY");
    Serial.println("========================================");
    Serial.println("  BOOT Button (GPIO0): Cycle backlight mode");
    Serial.println("  Backlight Modes: MANUAL -> AUTO -> OFF");
    Serial.println("  Auto Mode: LDR-based brightness");
    Serial.println("  Idle Timeout: 30s -> IDLE state");
    Serial.println("  Sleep Timeout: 5min -> Light Sleep");
    Serial.println("  Wakeup: BOOT button or Touch\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
