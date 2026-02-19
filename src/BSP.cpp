#include "BSP.h"
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

static TFT_eSPI tft = TFT_eSPI();
static SPIClass touchSPI;
XPT2046_Touchscreen* touchscreen = nullptr;

static bool displayReady = false;
static bool touchReady = false;
static uint8_t currentBacklight = 255;

#define VDB_BUFFER_SIZE (BSP_DISPLAY_WIDTH * 20)
static lv_color_t buf1[VDB_BUFFER_SIZE];
static lv_color_t buf2[VDB_BUFFER_SIZE];
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

static int16_t lastX = 0;
static int16_t lastY = 0;

static void initPWM() {
    ledcSetup(0, 5000, 8);
    ledcAttachPin(BSP_BACKLIGHT_PIN, 0);
}

void IRAM_ATTR bsp_display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t*)&color_p->full, w * h, false);
    tft.endWrite();
    
    lv_disp_flush_ready(disp);
}

void bsp_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    if (!touchscreen) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }
    
    if (touchscreen->tirqTouched() && touchscreen->touched()) {
        TS_Point p = touchscreen->getPoint();
        
        int16_t x = map(p.x, 200, 3700, 1, BSP_DISPLAY_WIDTH);
        int16_t y = map(p.y, 240, 3800, 1, BSP_DISPLAY_HEIGHT);
        
        x = constrain(x, 0, BSP_DISPLAY_WIDTH - 1);
        y = constrain(y, 0, BSP_DISPLAY_HEIGHT - 1);
        
        lastX = x;
        lastY = y;
        
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->point.x = lastX;
        data->point.y = lastY;
        data->state = LV_INDEV_STATE_REL;
    }
}

bool bsp_display_init(void) {
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, VDB_BUFFER_SIZE);
    
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = BSP_DISPLAY_WIDTH;
    disp_drv.ver_res = BSP_DISPLAY_HEIGHT;
    disp_drv.flush_cb = bsp_display_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    displayReady = true;
    return true;
}

bool bsp_touch_init(void) {
    touchSPI.begin(14, 12, 13, 33);
    
    touchscreen = new XPT2046_Touchscreen(33, 36);
    
    if (!touchscreen->begin(touchSPI)) {
        return false;
    }
    
    touchscreen->setRotation(1);
    
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = bsp_touch_read;
    lv_indev_drv_register(&indev_drv);
    
    touchReady = true;
    return true;
}

void bsp_init(void) {
    pinMode(BSP_LED_RED, OUTPUT);
    pinMode(BSP_LED_GREEN, OUTPUT);
    pinMode(BSP_LED_BLUE, OUTPUT);
    digitalWrite(BSP_LED_RED, HIGH);
    digitalWrite(BSP_LED_GREEN, HIGH);
    digitalWrite(BSP_LED_BLUE, HIGH);
    
    bsp_display_init();
    initPWM();
    bsp_touch_init();
    bsp_backlight_set(255);
}

void bsp_backlight_set(uint8_t level) {
    ledcWrite(0, level);
    currentBacklight = level;
}

uint8_t bsp_backlight_get(void) {
    return currentBacklight;
}

void bsp_rgb_led_set(uint8_t r, uint8_t g, uint8_t b) {
    digitalWrite(BSP_LED_RED, r > 0 ? LOW : HIGH);
    digitalWrite(BSP_LED_GREEN, g > 0 ? LOW : HIGH);
    digitalWrite(BSP_LED_BLUE, b > 0 ? LOW : HIGH);
}

void bsp_rgb_led_off(void) {
    digitalWrite(BSP_LED_RED, HIGH);
    digitalWrite(BSP_LED_GREEN, HIGH);
    digitalWrite(BSP_LED_BLUE, HIGH);
}

uint16_t bsp_light_sensor_read(void) {
    return analogRead(BSP_LDR_PIN);
}

uint32_t bsp_get_free_heap(void) {
    return ESP.getFreeHeap();
}

bool bsp_is_display_ready(void) {
    return displayReady;
}

bool bsp_is_touch_ready(void) {
    return touchReady;
}
