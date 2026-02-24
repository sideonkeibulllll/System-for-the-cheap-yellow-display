#include "BSP.h"
#include "ConfigManager.h"
#include "Storage.h"
#include "PowerManager.h"
#include "Performance.h"
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPIFFS.h>

static TFT_eSPI tft = TFT_eSPI();
static SPIClass touchSPI;
XPT2046_Touchscreen* touchscreen = nullptr;
static SPIClass sdSPI(VSPI);

static bool displayReady = false;
static bool touchReady = false;
static bool sdReady = false;
static bool spiffsReady = false;
static uint8_t currentBacklight = 255;

#define VDB_BUFFER_SIZE (BSP_DISPLAY_WIDTH * 10)
static lv_color_t buf1[VDB_BUFFER_SIZE];
static lv_color_t* buf2 = nullptr;
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

static int16_t lastX = 0;
static int16_t lastY = 0;
static bool touchFpsOptimizeEnabled = false;

static void initPWM() {
    Serial.println("[BSP] Initializing PWM backlight on GPIO 21...");
    
    ledcSetup(0, 5000, 8);
    ledcAttachPin(BSP_BACKLIGHT_PIN, 0);
    
    Serial.printf("  PWM frequency: 5000 Hz, 8-bit resolution\n");
    Serial.printf("  Channel: 0, Pin: %d\n", BSP_BACKLIGHT_PIN);
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
        
        if (touchFpsOptimizeEnabled) {
            Perf.setRefreshInterval(200);
        }
        
        Power.resetIdleTimer();
    } else {
        data->point.x = lastX;
        data->point.y = lastY;
        data->state = LV_INDEV_STATE_REL;
        
        if (touchFpsOptimizeEnabled) {
            Perf.setRefreshInterval(5);
        }
    }
}

void bsp_set_touch_fps_optimize(bool enable) {
    touchFpsOptimizeEnabled = enable;
}

bool bsp_display_init(void) {
    Serial.println("[BSP] Initializing display...");
    
    DisplayConfig& cfg = Config.getDisplayConfig();
    
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    if (cfg.invertColor) {
        tft.invertDisplay(true);
    }
    
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, VDB_BUFFER_SIZE);
    
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = BSP_DISPLAY_WIDTH;
    disp_drv.ver_res = BSP_DISPLAY_HEIGHT;
    disp_drv.flush_cb = bsp_display_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    displayReady = true;
    Serial.printf("  VDB buffer: %d pixels (double buffer)\n", VDB_BUFFER_SIZE);
    Serial.printf("  VDB size: %d bytes per buffer\n", VDB_BUFFER_SIZE * sizeof(lv_color_t));
    Serial.println("  Display init: OK");
    return true;
}

bool bsp_touch_init(void) {
    Serial.println("[BSP] Initializing touch (custom SPI)...");
    
    TouchConfig& cfg = Config.getTouchConfig();
    
    Serial.printf("  Touch pins: MOSI=%d, MISO=%d, CLK=%d, CS=%d, IRQ=%d\n",
        cfg.spiMosi, cfg.spiMiso, cfg.spiClk, cfg.spiCs, cfg.pinIrq);
    
    touchSPI.begin(cfg.spiClk, cfg.spiMiso, cfg.spiMosi, cfg.spiCs);
    
    touchscreen = new XPT2046_Touchscreen(cfg.spiCs, cfg.pinIrq);
    
    if (!touchscreen->begin(touchSPI)) {
        Serial.println("  Touch init: FAILED");
        return false;
    }
    
    touchscreen->setRotation(1);
    
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = bsp_touch_read;
    lv_indev_drv_register(&indev_drv);
    
    touchReady = true;
    Serial.println("  Touch init: OK");
    return true;
}

bool bsp_sd_init(void) {
    Serial.println("[BSP] Initializing SD card (hardware VSPI)...");
    
    StorageConfig& cfg = Config.getStorageConfig();
    
    sdSPI.begin(cfg.sdSpiClk, cfg.sdSpiMiso, cfg.sdSpiMosi, cfg.sdSpiCs);
    
    if (!SD.begin(cfg.sdSpiCs, sdSPI)) {
        Serial.println("  SD card: NOT DETECTED");
        sdReady = false;
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("  SD card: No card found");
        sdReady = false;
        return false;
    }
    
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("  SD card: %llu MB\n", cardSize);
    
    sdReady = true;
    Serial.println("  SD card init: OK");
    return true;
}

bool bsp_storage_init(void) {
    Serial.println("[BSP] Initializing Storage System...");
    
    if (Storage.begin()) {
        spiffsReady = Storage.isSPIFFSReady();
        sdReady = Storage.isSDReady();
        Serial.println("  Storage init: OK");
        return true;
    }
    
    Serial.println("  Storage init: PARTIAL (check individual systems)");
    spiffsReady = Storage.isSPIFFSReady();
    sdReady = Storage.isSDReady();
    return false;
}

bool bsp_is_spiffs_ready(void) {
    return spiffsReady;
}

void bsp_init(void) {
    Serial.println("\n[BSP] Board Support Package Init");
    Serial.println("=================================");
    
    pinMode(BSP_LED_RED, OUTPUT);
    pinMode(BSP_LED_GREEN, OUTPUT);
    pinMode(BSP_LED_BLUE, OUTPUT);
    digitalWrite(BSP_LED_RED, HIGH);
    digitalWrite(BSP_LED_GREEN, HIGH);
    digitalWrite(BSP_LED_BLUE, HIGH);
    
    bsp_display_init();
    
    initPWM();
    
    bsp_touch_init();
    bsp_storage_init();
    
    bsp_backlight_set(255);
    
    Serial.println("=================================");
    Serial.println("[BSP] Init complete\n");
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

bool bsp_is_sd_ready(void) {
    return sdReady;
}

void bsp_print_status(void) {
    Serial.println("\n[BSP] Status Report");
    Serial.println("-------------------");
    Serial.printf("  Display:  %s (%dx%d)\n", 
        displayReady ? "READY" : "NOT READY", BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT);
    Serial.printf("  Touch:    %s\n", touchReady ? "READY" : "NOT READY");
    Serial.printf("  SPIFFS:   %s\n", spiffsReady ? "READY" : "NOT READY");
    Serial.printf("  SD Card:  %s\n", sdReady ? "READY" : "NOT READY");
    Serial.printf("  Backlight: %d/255\n", currentBacklight);
    Serial.printf("  Free Heap: %u bytes\n", bsp_get_free_heap());
    Serial.println("-------------------\n");
}
