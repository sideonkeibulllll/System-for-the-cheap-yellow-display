#include "BSP.h"
#include "ConfigManager.h"
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <driver/ledc.h>

static TFT_eSPI tft = TFT_eSPI();
static SPIClass touchSPI = SPIClass(HSPI);
static XPT2046_Touchscreen* touchscreen = nullptr;
static SPIClass sdSPI = SPIClass(VSPI);

static bool displayReady = false;
static bool touchReady = false;
static bool sdReady = false;
static uint8_t currentBacklight = 255;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[BSP_DISPLAY_WIDTH * 20];
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

static int16_t touchLastX = 0;
static int16_t touchLastY = 0;
static bool touchLastPressed = false;

static bool debugTouch = true;
static unsigned long lastTouchDebug = 0;

static void initPWM() {
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);
    
    ledc_channel_config_t ledc_conf = {
        .gpio_num = (gpio_num_t)BSP_BACKLIGHT_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 255,
        .hpoint = 0,
        .flags = {.output_invert = 0}
    };
    ledc_channel_config(&ledc_conf);
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
    
    Serial.printf("  Display: %dx%d (landscape), SPI @ %d MHz\n", 
        BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT, cfg.spiFrequency / 1000000);
    
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, BSP_DISPLAY_WIDTH * 20);
    
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = BSP_DISPLAY_WIDTH;
    disp_drv.ver_res = BSP_DISPLAY_HEIGHT;
    disp_drv.flush_cb = bsp_display_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
    
    displayReady = true;
    Serial.println("  Display init: OK");
    return true;
}

bool bsp_touch_init(void) {
    Serial.println("[BSP] Initializing touch...");
    
    TouchConfig& cfg = Config.getTouchConfig();
    
    Serial.printf("  Touch SPI: MOSI=%d, MISO=%d, CLK=%d, CS=%d, IRQ=%d\n",
        cfg.spiMosi, cfg.spiMiso, cfg.spiClk, cfg.spiCs, cfg.pinIrq);
    
    touchSPI.begin(cfg.spiClk, cfg.spiMiso, cfg.spiMosi, cfg.spiCs);
    
    touchscreen = new XPT2046_Touchscreen(cfg.spiCs, cfg.pinIrq);
    if (!touchscreen->begin(touchSPI)) {
        Serial.println("  Touch init: FAILED");
        return false;
    }
    
    touchscreen->setRotation(1);
    
    Serial.println("  Touch init: OK");
    Serial.println("  Touch the screen to test...");
    
    touchReady = true;
    return true;
}

bool bsp_sd_init(void) {
    Serial.println("[BSP] Initializing SD card...");
    
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
    Serial.printf("  SD card: %llu MB, Type: ", cardSize);
    switch (cardType) {
        case CARD_MMC:  Serial.println("MMC"); break;
        case CARD_SD:   Serial.println("SD"); break;
        case CARD_SDHC: Serial.println("SDHC"); break;
        default:        Serial.println("Unknown"); break;
    }
    
    sdReady = true;
    Serial.println("  SD card init: OK");
    return true;
}

void bsp_init(void) {
    Serial.println("\n[BSP] Board Support Package Init");
    Serial.println("=================================");
    
    initPWM();
    
    pinMode(BSP_LED_RED, OUTPUT);
    pinMode(BSP_LED_GREEN, OUTPUT);
    pinMode(BSP_LED_BLUE, OUTPUT);
    digitalWrite(BSP_LED_RED, HIGH);
    digitalWrite(BSP_LED_GREEN, HIGH);
    digitalWrite(BSP_LED_BLUE, HIGH);
    
    bsp_display_init();
    bsp_touch_init();
    bsp_sd_init();
    
    bsp_backlight_set(255);
    
    Serial.println("=================================");
    Serial.println("[BSP] Init complete\n");
}

void IRAM_ATTR bsp_display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t*)&color_p->full, w * h, true);
    tft.endWrite();
    
    lv_disp_flush_ready(disp);
}

void bsp_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
    if (!touchscreen) {
        data->state = LV_INDEV_STATE_REL;
        return;
    }
    
    bool touched = touchscreen->touched();
    
    if (touched) {
        TS_Point p = touchscreen->getPoint();
        
        int16_t x = map(p.x, 200, 3900, 0, BSP_DISPLAY_WIDTH);
        int16_t y = map(p.y, 200, 3900, 0, BSP_DISPLAY_HEIGHT);
        
        x = constrain(x, 0, BSP_DISPLAY_WIDTH - 1);
        y = constrain(y, 0, BSP_DISPLAY_HEIGHT - 1);
        
        touchLastX = x;
        touchLastY = y;
        touchLastPressed = true;
        
        data->point.x = x;
        data->point.y = y;
        data->state = LV_INDEV_STATE_PR;
        
        if (debugTouch && (millis() - lastTouchDebug > 500)) {
            Serial.printf("[Touch] Raw: (%d, %d) -> Mapped: (%d, %d)\n", 
                p.x, p.y, x, y);
            lastTouchDebug = millis();
        }
    } else {
        data->point.x = touchLastX;
        data->point.y = touchLastY;
        data->state = LV_INDEV_STATE_REL;
        touchLastPressed = false;
    }
}

void bsp_backlight_set(uint8_t level) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, level);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
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
    Serial.printf("  SD Card:  %s\n", sdReady ? "READY" : "NOT READY");
    Serial.printf("  Backlight: %d/255\n", currentBacklight);
    Serial.printf("  Free Heap: %u bytes\n", bsp_get_free_heap());
    Serial.println("-------------------\n");
}
