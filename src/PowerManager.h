#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_pm.h>
#include <driver/adc.h>

#define POWER_BOOT_PIN          0
#define POWER_LDR_PIN           34
#define POWER_BACKLIGHT_PIN     21

#define POWER_BACKLIGHT_MAX     255
#define POWER_BACKLIGHT_MIN     10
#define POWER_BACKLIGHT_DIM     50

#define POWER_IDLE_TIMEOUT_MS       30000
#define POWER_SLEEP_TIMEOUT_MS      300000
#define POWER_AUTO_ADJUST_INTERVAL  1000

typedef enum {
    POWER_MODE_HIGH = 0,
    POWER_MODE_BALANCED = 1,
    POWER_MODE_LOW = 2
} power_cpu_mode_t;

typedef enum {
    BACKLIGHT_MODE_MANUAL = 0,
    BACKLIGHT_MODE_AUTO = 1,
    BACKLIGHT_MODE_OFF = 2
} backlight_mode_t;

typedef enum {
    POWER_STATE_ACTIVE = 0,
    POWER_STATE_IDLE = 1,
    POWER_STATE_SLEEP = 2
} power_state_t;

typedef struct {
    power_cpu_mode_t cpuMode;
    backlight_mode_t backlightMode;
    power_state_t state;
    uint8_t backlightLevel;
    uint16_t ldrValue;
    uint32_t idleTimeMs;
    uint32_t lastActivityMs;
    bool bootButtonPressed;
    uint32_t bootPressCount;
} power_status_t;

typedef void (*power_state_callback_t)(power_state_t newState);
typedef void (*backlight_mode_callback_t)(backlight_mode_t newMode);

class PowerManager {
public:
    PowerManager();
    ~PowerManager();
    
    bool begin();
    void end();
    
    void update();
    
    void setCpuMode(power_cpu_mode_t mode);
    power_cpu_mode_t getCpuMode() { return _status.cpuMode; }
    
    void setBacklightMode(backlight_mode_t mode);
    backlight_mode_t getBacklightMode() { return _status.backlightMode; }
    
    void setBacklight(uint8_t level);
    uint8_t getBacklight() { return _status.backlightLevel; }
    
    void toggleBacklight();
    void cycleBacklightMode();
    
    void setAutoBacklightParams(uint8_t minLevel, uint8_t maxLevel);
    
    void forceSleep();
    void forceWakeup();
    
    power_state_t getState() { return _status.state; }
    power_status_t getStatus() { return _status; }
    
    void resetIdleTimer();
    uint32_t getIdleTimeMs() { return _status.idleTimeMs; }
    
    void setStateCallback(power_state_callback_t cb) { _stateCallback = cb; }
    void setBacklightModeCallback(backlight_mode_callback_t cb) { _backlightModeCallback = cb; }
    
    void printStatus();
    
    static void IRAM_ATTR bootButtonISR();
    static void IRAM_ATTR touchISR();
    
private:
    bool _initialized;
    power_status_t _status;
    
    uint8_t _autoMinLevel;
    uint8_t _autoMaxLevel;
    uint32_t _lastAutoAdjustMs;
    uint32_t _lastUpdateMs;
    
    volatile bool _bootButtonFlag;
    volatile uint32_t _bootButtonDebounceMs;
    
    power_state_callback_t _stateCallback;
    backlight_mode_callback_t _backlightModeCallback;
    
    void updateBacklightAuto();
    void updateIdleState();
    void updateCpuFrequency();
    
    void enterSleep();
    void exitSleep();
    
    void handleBootButton();
    
    uint8_t calculateBacklightFromLDR(uint16_t ldrValue);
};

extern PowerManager Power;

#endif
