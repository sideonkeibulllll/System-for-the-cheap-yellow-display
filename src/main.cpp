#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "ConfigManager.h"
#include "BSP.h"
#include "Storage.h"
#include "Performance.h"

static lv_obj_t* labelStatus;
static lv_obj_t* labelPerf;
static lv_obj_t* btnTest;
static int clickCount = 0;

static TaskHandle_t appTaskHandle = nullptr;

static void btn_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        clickCount++;
        lv_label_set_text_fmt(labelStatus, "Clicked %d times", clickCount);
        bsp_rgb_led_set(rand() % 256, rand() % 256, rand() % 256);
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

void createUI() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Stage 5: Performance");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, 
        "Dual Core: Core1=LVGL\n"
        "Double Buffer: 6.4KBx2\n"
        "LVGL Memory: 48KB\n"
        "Tick: Hardware Timer 1ms"
    );
    lv_obj_set_style_text_color(info, lv_color_white(), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_12, 0);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 5, 35);
    
    btnTest = lv_btn_create(scr);
    lv_obj_set_size(btnTest, 140, 45);
    lv_obj_align(btnTest, LV_ALIGN_CENTER, 0, 10);
    lv_obj_add_event_cb(btnTest, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(btnTest, lv_color_make(0x00, 0x80, 0xFF), 0);
    
    lv_obj_t* btnLabel = lv_label_create(btnTest);
    lv_label_set_text(btnLabel, "Touch Me!");
    lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(btnLabel);
    
    labelStatus = lv_label_create(scr);
    lv_label_set_text(labelStatus, "Performance optimized!");
    lv_obj_set_style_text_color(labelStatus, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_14, 0);
    lv_obj_align(labelStatus, LV_ALIGN_BOTTOM_MID, 0, -45);
    
    labelPerf = lv_label_create(scr);
    lv_label_set_text(labelPerf, "FPS: -- | Heap: -- KB");
    lv_obj_set_style_text_color(labelPerf, lv_color_make(0x00, 0xFF, 0xFF), 0);
    lv_obj_set_style_text_font(labelPerf, &lv_font_montserrat_12, 0);
    lv_obj_align(labelPerf, LV_ALIGN_BOTTOM_MID, 0, -10);
    
    lv_timer_create(perf_update_cb, 100, NULL);
}

static void appTaskEntry(void* arg) {
    Serial.println("[App] Application task running on Core 0");
    
    while (true) {
        static uint32_t lastPrint = 0;
        uint32_t now = millis();
        
        if (now - lastPrint >= 5000) {
            lastPrint = now;
            Perf.printStats();
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n========================================");
    Serial.println("  ESP32-2432S028R (CYD) v3 - Stage 5");
    Serial.println("  LVGL Performance Optimization");
    Serial.println("========================================\n");
    
    Serial.printf("Initial CPU Freq: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("Running on Core: %d\n", xPortGetCoreID());
    
    Config.begin();
    lv_init();
    
    Perf.begin();
    
    bsp_init();
    
    bsp_rgb_led_set(255, 0, 0);
    delay(200);
    bsp_rgb_led_set(0, 255, 0);
    delay(200);
    bsp_rgb_led_set(0, 0, 255);
    delay(200);
    bsp_rgb_led_off();
    
    bsp_print_status();
    Storage.printStatus();
    
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
    Serial.println("  Stage 5: Performance Optimization READY");
    Serial.println("========================================");
    Serial.println("  Core 0: Application tasks");
    Serial.println("  Core 1: LVGL rendering (5ms tick)");
    Serial.println("  Hardware Timer: 1ms period");
    Serial.println("  Double Buffer: 6.4KB x 2");
    Serial.println("  LVGL Memory Pool: 48KB\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
