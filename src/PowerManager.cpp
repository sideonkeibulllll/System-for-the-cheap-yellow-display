#include "PowerManager.h"
#include "BSP.h"
#include <driver/ledc.h>

PowerManager Power;

static volatile bool bootButtonPressed = false;
static volatile bool touchInterruptFlag = false;

PowerManager::PowerManager() 
    : _initialized(false)
    , _autoMinLevel(POWER_BACKLIGHT_MIN)
    , _autoMaxLevel(POWER_BACKLIGHT_MAX)
    , _lastAutoAdjustMs(0)
    , _lastUpdateMs(0)
    , _bootButtonFlag(false)
    , _bootButtonDebounceMs(0)
    , _stateCallback(nullptr)
    , _backlightModeCallback(nullptr)
{
    memset(&_status, 0, sizeof(_status));
    _status.cpuMode = POWER_MODE_HIGH;
    _status.backlightMode = BACKLIGHT_MODE_MANUAL;
    _status.state = POWER_STATE_ACTIVE;
    _status.backlightLevel = POWER_BACKLIGHT_MAX;
}

PowerManager::~PowerManager() {
    end();
}

void IRAM_ATTR PowerManager::bootButtonISR() {
    bootButtonPressed = true;
}

void IRAM_ATTR PowerManager::touchISR() {
    touchInterruptFlag = true;
}

bool PowerManager::begin() {
    if (_initialized) {
        return true;
    }
    
    Serial.println("[Power] Initializing Power Manager...");
    
    pinMode(POWER_BOOT_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(POWER_BOOT_PIN), bootButtonISR, FALLING);
    
    pinMode(POWER_LDR_PIN, INPUT);
    
    _status.cpuMode = POWER_MODE_HIGH;
    _status.backlightMode = BACKLIGHT_MODE_MANUAL;
    _status.state = POWER_STATE_ACTIVE;
    _status.backlightLevel = POWER_BACKLIGHT_MAX;
    _status.lastActivityMs = millis();
    _status.bootPressCount = 0;
    
    bsp_backlight_set(_status.backlightLevel);
    
    _initialized = true;
    _lastUpdateMs = millis();
    
    Serial.println("[Power] Power Manager initialized");
    Serial.printf("  BOOT Button: GPIO %d (interrupt enabled)\n", POWER_BOOT_PIN);
    Serial.printf("  LDR Sensor: GPIO %d\n", POWER_LDR_PIN);
    Serial.printf("  Backlight: GPIO %d\n", POWER_BACKLIGHT_PIN);
    Serial.printf("  Initial CPU Mode: %d MHz\n", getCpuFrequencyMhz());
    
    return true;
}

void PowerManager::end() {
    if (!_initialized) {
        return;
    }
    
    detachInterrupt(digitalPinToInterrupt(POWER_BOOT_PIN));
    _initialized = false;
}

void PowerManager::update() {
    if (!_initialized) {
        return;
    }
    
    uint32_t now = millis();
    _status.idleTimeMs = now - _status.lastActivityMs;
    
    handleBootButton();
    
    if (_status.backlightMode == BACKLIGHT_MODE_AUTO) {
        if (now - _lastAutoAdjustMs >= POWER_AUTO_ADJUST_INTERVAL) {
            _lastAutoAdjustMs = now;
            updateBacklightAuto();
        }
    }
    
    updateIdleState();
}

void PowerManager::handleBootButton() {
    if (bootButtonPressed) {
        bootButtonPressed = false;
        
        uint32_t now = millis();
        if (now - _bootButtonDebounceMs < 200) {
            return;
        }
        _bootButtonDebounceMs = now;
        
        _status.bootPressCount++;
        _status.bootButtonPressed = true;
        
        if (_status.state == POWER_STATE_SLEEP) {
            Serial.println("[Power] BOOT button pressed - waking up");
            exitSleep();
        } else {
            Serial.printf("[Power] BOOT button pressed - cycling backlight mode (%u)\n", _status.bootPressCount);
            cycleBacklightMode();
        }
        
        resetIdleTimer();
        
        _status.bootButtonPressed = false;
    }
}

void PowerManager::cycleBacklightMode() {
    backlight_mode_t newMode;
    
    switch (_status.backlightMode) {
        case BACKLIGHT_MODE_MANUAL:
            newMode = BACKLIGHT_MODE_AUTO;
            Serial.println("[Power] Backlight: MANUAL -> AUTO");
            break;
        case BACKLIGHT_MODE_AUTO:
            newMode = BACKLIGHT_MODE_OFF;
            Serial.println("[Power] Backlight: AUTO -> OFF");
            break;
        case BACKLIGHT_MODE_OFF:
        default:
            newMode = BACKLIGHT_MODE_MANUAL;
            _status.backlightLevel = POWER_BACKLIGHT_MAX;
            Serial.println("[Power] Backlight: OFF -> MANUAL");
            break;
    }
    
    setBacklightMode(newMode);
}

void PowerManager::setBacklightMode(backlight_mode_t mode) {
    _status.backlightMode = mode;
    
    switch (mode) {
        case BACKLIGHT_MODE_MANUAL:
            setBacklight(_status.backlightLevel > 0 ? _status.backlightLevel : POWER_BACKLIGHT_MAX);
            break;
        case BACKLIGHT_MODE_AUTO:
            updateBacklightAuto();
            break;
        case BACKLIGHT_MODE_OFF:
            bsp_backlight_set(0);
            break;
    }
    
    if (_backlightModeCallback) {
        _backlightModeCallback(mode);
    }
}

void PowerManager::setBacklight(uint8_t level) {
    if (_status.backlightMode == BACKLIGHT_MODE_OFF && level > 0) {
        Serial.println("[Power] setBacklight ignored: OFF mode active");
        return;
    }
    _status.backlightLevel = level;
    bsp_backlight_set(level);
}

void PowerManager::toggleBacklight() {
    if (_status.backlightLevel > 0) {
        setBacklight(0);
    } else {
        setBacklight(POWER_BACKLIGHT_MAX);
    }
}

void PowerManager::setAutoBacklightParams(uint8_t minLevel, uint8_t maxLevel) {
    _autoMinLevel = minLevel;
    _autoMaxLevel = maxLevel;
}

void PowerManager::updateBacklightAuto() {
    _status.ldrValue = analogRead(POWER_LDR_PIN);
    
    uint8_t newLevel = calculateBacklightFromLDR(_status.ldrValue);
    
    if (newLevel != _status.backlightLevel) {
        setBacklight(newLevel);
    }
}

uint8_t PowerManager::calculateBacklightFromLDR(uint16_t ldrValue) {
    uint16_t minLDR = 500;
    uint16_t maxLDR = 3500;
    
    ldrValue = constrain(ldrValue, minLDR, maxLDR);
    
    uint16_t normalized = map(ldrValue, minLDR, maxLDR, 0, 255);
    
    uint8_t level = map(normalized, 0, 255, _autoMinLevel, _autoMaxLevel);
    
    return level;
}

void PowerManager::setCpuMode(power_cpu_mode_t mode) {
    _status.cpuMode = mode;
    updateCpuFrequency();
}

void PowerManager::updateCpuFrequency() {
    uint32_t targetFreq = 240;
    
    switch (_status.cpuMode) {
        case POWER_MODE_HIGH:
            targetFreq = 240;
            break;
        case POWER_MODE_BALANCED:
            targetFreq = 160;
            break;
        case POWER_MODE_LOW:
            targetFreq = 80;
            break;
    }
    
    if (getCpuFrequencyMhz() != targetFreq) {
        setCpuFrequencyMhz(targetFreq);
        Serial.printf("[Power] CPU frequency changed to %d MHz\n", targetFreq);
    }
}

void PowerManager::updateIdleState() {
    uint32_t idleMs = _status.idleTimeMs;
    
    if (_status.state == POWER_STATE_SLEEP) {
        return;
    }
    
    if (idleMs >= POWER_SLEEP_TIMEOUT_MS) {
        if (_status.state != POWER_STATE_SLEEP) {
            enterSleep();
        }
    } else if (idleMs >= POWER_IDLE_TIMEOUT_MS) {
        if (_status.state == POWER_STATE_ACTIVE) {
            _status.state = POWER_STATE_IDLE;
            setCpuMode(POWER_MODE_BALANCED);
            
            if (_status.backlightMode == BACKLIGHT_MODE_MANUAL) {
                bsp_backlight_set(POWER_BACKLIGHT_DIM);
                _status.backlightLevel = POWER_BACKLIGHT_DIM;
            } else if (_status.backlightMode == BACKLIGHT_MODE_AUTO) {
                uint8_t dimLevel = (_autoMinLevel + POWER_BACKLIGHT_DIM) / 2;
                bsp_backlight_set(dimLevel);
                _status.backlightLevel = dimLevel;
            }
            
            Serial.println("[Power] Entering IDLE state");
            
            if (_stateCallback) {
                _stateCallback(POWER_STATE_IDLE);
            }
        }
    } else {
        if (_status.state == POWER_STATE_IDLE) {
            _status.state = POWER_STATE_ACTIVE;
            setCpuMode(POWER_MODE_HIGH);
            
            if (_status.backlightMode == BACKLIGHT_MODE_MANUAL) {
                bsp_backlight_set(POWER_BACKLIGHT_MAX);
                _status.backlightLevel = POWER_BACKLIGHT_MAX;
            } else if (_status.backlightMode == BACKLIGHT_MODE_AUTO) {
                updateBacklightAuto();
            }
            
            Serial.println("[Power] Returning to ACTIVE state");
            
            if (_stateCallback) {
                _stateCallback(POWER_STATE_ACTIVE);
            }
        }
    }
}

void PowerManager::enterSleep() {
    Serial.println("[Power] Entering SLEEP state...");
    
    _status.state = POWER_STATE_SLEEP;
    
    bsp_backlight_set(0);
    
    setCpuMode(POWER_MODE_LOW);
    
    if (_stateCallback) {
        _stateCallback(POWER_STATE_SLEEP);
    }
    
    Serial.println("[Power] Entering light sleep...");
    Serial.flush();
    
    esp_sleep_enable_ext0_wakeup((gpio_num_t)POWER_BOOT_PIN, LOW);
    
    esp_sleep_enable_timer_wakeup(60 * 1000000);
    
    esp_light_sleep_start();
    
    exitSleep();
}

void PowerManager::exitSleep() {
    Serial.println("[Power] Waking up from sleep...");
    
    _status.state = POWER_STATE_ACTIVE;
    _status.lastActivityMs = millis();
    
    setCpuMode(POWER_MODE_HIGH);
    
    if (_status.backlightMode == BACKLIGHT_MODE_MANUAL) {
        bsp_backlight_set(POWER_BACKLIGHT_MAX);
        _status.backlightLevel = POWER_BACKLIGHT_MAX;
    } else if (_status.backlightMode == BACKLIGHT_MODE_AUTO) {
        updateBacklightAuto();
    } else {
        _status.backlightMode = BACKLIGHT_MODE_MANUAL;
        bsp_backlight_set(POWER_BACKLIGHT_MAX);
        _status.backlightLevel = POWER_BACKLIGHT_MAX;
    }
    
    Serial.println("[Power] Wakeup complete - ACTIVE state");
    
    if (_stateCallback) {
        _stateCallback(POWER_STATE_ACTIVE);
    }
}

void PowerManager::forceSleep() {
    enterSleep();
}

void PowerManager::forceWakeup() {
    if (_status.state == POWER_STATE_SLEEP) {
        exitSleep();
    }
}

void PowerManager::resetIdleTimer() {
    _status.lastActivityMs = millis();
    _status.idleTimeMs = 0;
    
    if (_status.state == POWER_STATE_IDLE) {
        _status.state = POWER_STATE_ACTIVE;
        setCpuMode(POWER_MODE_HIGH);
        
        if (_status.backlightMode == BACKLIGHT_MODE_MANUAL) {
            setBacklight(POWER_BACKLIGHT_MAX);
        } else if (_status.backlightMode == BACKLIGHT_MODE_AUTO) {
            updateBacklightAuto();
        }
        
        Serial.println("[Power] Activity detected - ACTIVE state");
        
        if (_stateCallback) {
            _stateCallback(POWER_STATE_ACTIVE);
        }
    }
}

void PowerManager::printStatus() {
    Serial.println("\n[Power] Power Manager Status");
    Serial.println("============================");
    Serial.printf("  State:          %s\n", 
        _status.state == POWER_STATE_ACTIVE ? "ACTIVE" :
        _status.state == POWER_STATE_IDLE ? "IDLE" : "SLEEP");
    Serial.printf("  CPU Mode:       %s (%d MHz)\n",
        _status.cpuMode == POWER_MODE_HIGH ? "HIGH" :
        _status.cpuMode == POWER_MODE_BALANCED ? "BALANCED" : "LOW",
        getCpuFrequencyMhz());
    Serial.printf("  Backlight Mode: %s\n",
        _status.backlightMode == BACKLIGHT_MODE_MANUAL ? "MANUAL" :
        _status.backlightMode == BACKLIGHT_MODE_AUTO ? "AUTO" : "OFF");
    Serial.printf("  Backlight:      %d / %d\n", _status.backlightLevel, POWER_BACKLIGHT_MAX);
    Serial.printf("  LDR Value:      %d\n", _status.ldrValue);
    Serial.printf("  Idle Time:      %u ms\n", _status.idleTimeMs);
    Serial.printf("  BOOT Presses:   %u\n", _status.bootPressCount);
    Serial.println("============================\n");
}
