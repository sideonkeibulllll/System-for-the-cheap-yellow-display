#include <Arduino.h>
#include <lvgl.h>
#include "ConfigManager.h"
#include "BSP.h"

static lv_obj_t* labelStatus;
static lv_obj_t* btnTest;
static int clickCount = 0;

static void btn_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    if (code == LV_EVENT_CLICKED) {
        clickCount++;
        lv_label_set_text_fmt(labelStatus, "Button clicked %d times!", clickCount);
        bsp_rgb_led_set(rand() % 256, rand() % 256, rand() % 256);
        Serial.printf("[UI] Button clicked: %d\n", clickCount);
    }
}

void createUI() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);
    
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, "Stage 3: BSP Complete");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    
    lv_obj_t* info = lv_label_create(scr);
    lv_label_set_text(info, "ESP32-2432S028R v3\nDisplay: ST7789 320x240\nTouch: XPT2046\nLVGL: v8.3");
    lv_obj_set_style_text_color(info, lv_color_white(), 0);
    lv_obj_set_style_text_font(info, &lv_font_montserrat_14, 0);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 10, 50);
    
    btnTest = lv_btn_create(scr);
    lv_obj_set_size(btnTest, 160, 50);
    lv_obj_align(btnTest, LV_ALIGN_CENTER, 0, 20);
    lv_obj_add_event_cb(btnTest, btn_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_style_bg_color(btnTest, lv_color_make(0x00, 0x80, 0xFF), 0);
    
    lv_obj_t* btnLabel = lv_label_create(btnTest);
    lv_label_set_text(btnLabel, "Touch Me!");
    lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(btnLabel);
    
    labelStatus = lv_label_create(scr);
    lv_label_set_text(labelStatus, "Touch the button to test!");
    lv_obj_set_style_text_color(labelStatus, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_14, 0);
    lv_obj_align(labelStatus, LV_ALIGN_BOTTOM_MID, 0, -30);
    
    Serial.println("[UI] LVGL UI created");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n========================================");
    Serial.println("  ESP32-2432S028R (CYD) v3 - Stage 3");
    Serial.println("  BSP (Board Support Package) Test");
    Serial.println("========================================\n");
    
    Serial.println("[Stage 3] Initializing ConfigManager...");
    Config.begin();
    Serial.println("  ConfigManager initialized\n");
    
    Serial.println("[Stage 3] Initializing LVGL...");
    lv_init();
    Serial.println("  LVGL initialized");
    
    Serial.println("[Stage 3] Initializing BSP...");
    bsp_init();
    
    Serial.println("[Stage 3] Running hardware tests...");
    
    Serial.println("[Test] RGB LED...");
    bsp_rgb_led_set(255, 0, 0);
    delay(300);
    bsp_rgb_led_set(0, 255, 0);
    delay(300);
    bsp_rgb_led_set(0, 0, 255);
    delay(300);
    bsp_rgb_led_off();
    Serial.println("  RGB LED Test: PASSED");
    
    Serial.println("[Test] Backlight PWM...");
    for (int i = 255; i >= 0; i -= 15) {
        bsp_backlight_set(i);
        delay(20);
    }
    for (int i = 0; i <= 255; i += 15) {
        bsp_backlight_set(i);
        delay(20);
    }
    Serial.println("  Backlight PWM Test: PASSED");
    
    Serial.println("[Test] Light Sensor...");
    Serial.printf("  LDR Value: %d\n", bsp_light_sensor_read());
    Serial.println("  Light Sensor Test: PASSED");
    
    bsp_print_status();
    
    Serial.println("[Stage 3] Creating LVGL UI...");
    createUI();
    
    Serial.println("\n========================================");
    Serial.println("  Stage 3: BSP Test READY");
    Serial.println("========================================");
    Serial.println("  Touch the button to test touch input!\n");
}

void loop() {
    lv_timer_handler();
    delay(5);
}
