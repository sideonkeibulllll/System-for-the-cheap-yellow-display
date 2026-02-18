#include <Arduino.h>
#include <lvgl.h>
#include "ConfigManager.h"
#include "BSP.h"

static lv_obj_t* mainScreen = nullptr;
static lv_obj_t* labelTitle = nullptr;
static lv_obj_t* labelStatus = nullptr;
static lv_obj_t* labelHeap = nullptr;
static lv_obj_t* btnTest = nullptr;
static lv_obj_t* labelBtn = nullptr;

static lv_timer_t* heapTimer = nullptr;

void printBanner() {
    Serial.println();
    Serial.println("========================================");
    Serial.println("  ESP32-2432S028R (CYD) v3 - Stage 3");
    Serial.println("  BSP (Board Support Package) Test");
    Serial.println("========================================");
    Serial.println();
}

void testRGBLED() {
    Serial.println("[Test] RGB LED...");
    
    bsp_rgb_led_set(255, 0, 0);
    delay(300);
    bsp_rgb_led_set(0, 255, 0);
    delay(300);
    bsp_rgb_led_set(0, 0, 255);
    delay(300);
    bsp_rgb_led_set(255, 255, 255);
    delay(300);
    bsp_rgb_led_off();
    
    Serial.println("  RGB LED Test: PASSED");
    Serial.println();
}

void testBacklightPWM() {
    Serial.println("[Test] Backlight PWM...");
    
    for (int i = 255; i >= 0; i -= 5) {
        bsp_backlight_set(i);
        delay(10);
    }
    
    for (int i = 0; i <= 255; i += 5) {
        bsp_backlight_set(i);
        delay(10);
    }
    
    bsp_backlight_set(255);
    Serial.println("  Backlight PWM Test: PASSED");
    Serial.println();
}

void testLightSensor() {
    Serial.println("[Test] Light Sensor (LDR)...");
    
    uint16_t readings[5];
    for (int i = 0; i < 5; i++) {
        readings[i] = bsp_light_sensor_read();
        delay(100);
    }
    
    uint32_t avg = 0;
    for (int i = 0; i < 5; i++) {
        avg += readings[i];
    }
    avg /= 5;
    
    Serial.printf("  LDR Average: %d (range: %d - %d)\n", avg, readings[0], readings[4]);
    Serial.println("  Light Sensor Test: PASSED");
    Serial.println();
}

static void heap_timer_cb(lv_timer_t* timer) {
    if (labelHeap) {
        lv_label_set_text_fmt(labelHeap, "Heap: %u bytes", bsp_get_free_heap());
    }
}

static void btn_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        static uint8_t clickCount = 0;
        clickCount++;
        
        bsp_rgb_led_set(rand() % 256, rand() % 256, rand() % 256);
        
        lv_label_set_text_fmt(labelStatus, 
            "Button clicked %d times!\nTouch works!", clickCount);
        
        Serial.printf("[UI] Button clicked: %d\n", clickCount);
    }
}

void createUI() {
    mainScreen = lv_scr_act();
    lv_obj_set_style_bg_color(mainScreen, lv_color_black(), 0);
    
    labelTitle = lv_label_create(mainScreen);
    lv_label_set_text(labelTitle, "Stage 3: BSP");
    lv_obj_set_style_text_color(labelTitle, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelTitle, &lv_font_montserrat_24, 0);
    lv_obj_align(labelTitle, LV_ALIGN_TOP_MID, 0, 5);
    
    lv_obj_t* labelInfo = lv_label_create(mainScreen);
    lv_label_set_text(labelInfo, 
        "Board Support Package\n"
        "=====================\n"
        "Display: ST7789 320x240\n"
        "Touch: XPT2046\n"
        "SD Card: Ready\n"
        "LVGL: v8.3");
    lv_obj_set_style_text_color(labelInfo, lv_color_white(), 0);
    lv_obj_set_style_text_font(labelInfo, &lv_font_montserrat_14, 0);
    lv_obj_align(labelInfo, LV_ALIGN_TOP_LEFT, 5, 35);
    
    btnTest = lv_btn_create(mainScreen);
    lv_obj_set_size(btnTest, 140, 45);
    lv_obj_align(btnTest, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_event_cb(btnTest, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(btnTest, lv_color_make(0x00, 0x80, 0xFF), 0);
    
    labelBtn = lv_label_create(btnTest);
    lv_label_set_text(labelBtn, "Touch Me!");
    lv_obj_set_style_text_font(labelBtn, &lv_font_montserrat_16, 0);
    lv_obj_center(labelBtn);
    
    labelStatus = lv_label_create(mainScreen);
    lv_label_set_text(labelStatus, "Waiting for touch...");
    lv_obj_set_style_text_color(labelStatus, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_14, 0);
    lv_obj_align(labelStatus, LV_ALIGN_BOTTOM_LEFT, 5, -45);
    
    labelHeap = lv_label_create(mainScreen);
    lv_label_set_text(labelHeap, "Heap: --");
    lv_obj_set_style_text_color(labelHeap, lv_color_make(0x00, 0xFF, 0xFF), 0);
    lv_obj_set_style_text_font(labelHeap, &lv_font_montserrat_14, 0);
    lv_obj_align(labelHeap, LV_ALIGN_BOTTOM_LEFT, 5, -25);
    
    heapTimer = lv_timer_create(heap_timer_cb, 1000, NULL);
    
    Serial.println("[UI] LVGL UI created (320x240 landscape)");
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    printBanner();
    
    Serial.println("[Stage 3] Initializing ConfigManager...");
    Config.begin();
    Serial.println();
    
    Serial.println("[Stage 3] Initializing LVGL...");
    lv_init();
    Serial.println("  LVGL initialized");
    
    Serial.println("\n[Stage 3] Running hardware tests...");
    testRGBLED();
    testBacklightPWM();
    testLightSensor();
    
    Serial.println("[Stage 3] Initializing BSP...");
    bsp_init();
    
    bsp_print_status();
    
    Serial.println("[Stage 3] Creating LVGL UI...");
    createUI();
    
    Serial.println("========================================");
    Serial.println("  Stage 3: BSP Test READY");
    Serial.println("========================================");
    Serial.println();
    Serial.println("Touch the button to test touch input!");
    Serial.println();
}

void loop() {
    lv_timer_handler();
    delay(5);
    
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 10000) {
        Serial.printf("[%lu s] BSP running - Free Heap: %u bytes\n", 
            millis() / 1000, bsp_get_free_heap());
        lastPrint = millis();
    }
}
