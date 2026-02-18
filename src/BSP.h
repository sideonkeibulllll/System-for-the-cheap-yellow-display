#ifndef BSP_H
#define BSP_H

#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <SD.h>

#define BSP_DISPLAY_WIDTH  320
#define BSP_DISPLAY_HEIGHT 240

#define BSP_LED_RED    4
#define BSP_LED_GREEN  16
#define BSP_LED_BLUE   17
#define BSP_LDR_PIN    34
#define BSP_BACKLIGHT_PIN 21

typedef struct {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
} bsp_touch_cal_matrix_t;

typedef enum {
    BSP_OK = 0,
    BSP_ERR_DISPLAY_INIT,
    BSP_ERR_TOUCH_INIT,
    BSP_ERR_SD_INIT,
    BSP_ERR_CONFIG_LOAD
} bsp_err_t;

void bsp_init(void);
bool bsp_display_init(void);
bool bsp_touch_init(void);
bool bsp_sd_init(void);

void bsp_backlight_set(uint8_t level);
uint8_t bsp_backlight_get(void);

void bsp_rgb_led_set(uint8_t r, uint8_t g, uint8_t b);
void bsp_rgb_led_off(void);

uint16_t bsp_light_sensor_read(void);

uint32_t bsp_get_free_heap(void);

bool bsp_is_display_ready(void);
bool bsp_is_touch_ready(void);
bool bsp_is_sd_ready(void);

void bsp_display_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void bsp_touch_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

void bsp_print_status(void);

#endif
