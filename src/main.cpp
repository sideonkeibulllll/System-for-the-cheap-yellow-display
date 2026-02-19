#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include "BSP.h"
#include "Performance.h"

static lv_obj_t* labelStatus;
static lv_obj_t* labelPerf;
static lv_timer_t* updateTimer = nullptr;

static void update_cb(lv_timer_t* timer) {
    if (labelPerf && lv_obj_is_valid(labelPerf)) {
        perf_stats_t stats = Perf.getStats();
        lv_label_set_text_fmt(labelPerf, 
            "FPS: %u | Heap: %u KB",
            stats.fps,
            stats.freeHeap / 1024
        );
    }
}

static void createUI() {
    lv_obj_t* scr = lv_scr_act();
    
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "ESP32-2432S028R");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
    
    lv_obj_t* subtitle = lv_label_create(scr);
    lv_label_set_text(subtitle, "Minimal System Ready");
    lv_obj_set_style_text_color(subtitle, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_MID, 0, 50);
    
    labelStatus = lv_label_create(scr);
    lv_label_set_text(labelStatus, "System: OK");
    lv_obj_set_style_text_color(labelStatus, lv_color_make(0x00, 0xCC, 0x00), 0);
    lv_obj_align(labelStatus, LV_ALIGN_CENTER, 0, -20);
    
    labelPerf = lv_label_create(scr);
    lv_label_set_text(labelPerf, "FPS: -- | Heap: -- KB");
    lv_obj_set_style_text_color(labelPerf, lv_color_make(0xDD, 0xDD, 0xDD), 0);
    lv_obj_align(labelPerf, LV_ALIGN_CENTER, 0, 20);
    
    lv_obj_t* hint = lv_label_create(scr);
    lv_label_set_text(hint, "High Performance Mode");
    lv_obj_set_style_text_color(hint, lv_color_make(0x99, 0x99, 0x99), 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    updateTimer = lv_timer_create(update_cb, 500, NULL);
}

void setup() {
    Serial.begin(115200);
    delay(500);
    
    Serial.println("\n========================================");
    Serial.println("  ESP32-2432S028R (CYD) - Minimal");
    Serial.println("========================================\n");
    
    lv_init();
    Perf.begin();
    bsp_init();
    
    bsp_rgb_led_set(0, 255, 0);
    delay(200);
    bsp_rgb_led_off();
    
    createUI();
    Perf.startLvglTask();
    
    Serial.println("System Ready");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}
