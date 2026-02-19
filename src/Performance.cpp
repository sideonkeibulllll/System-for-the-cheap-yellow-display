#include "Performance.h"
#include "BSP.h"
#include <lvgl.h>
#include <esp_pm.h>
#include <esp_sleep.h>

PerformanceManager Perf;

static volatile uint32_t tickCounter = 0;
static volatile bool lvglTaskRunning = false;

PerformanceManager::PerformanceManager() 
    : _initialized(false)
    , _currentMode(PERF_MODE_HIGH)
    , _lvglTaskHandle(nullptr)
    , _tickTimer(nullptr)
    , _tickInterval(PERF_TICK_INTERVAL_US)
{
    memset(&_stats, 0, sizeof(_stats));
}

PerformanceManager::~PerformanceManager() {
    end();
}

bool PerformanceManager::begin() {
    if (_initialized) {
        return true;
    }
    
    Serial.println("[Perf] Initializing Performance Manager...");
    
    _currentMode = PERF_MODE_HIGH;
    
    createTickTimer();
    
    _initialized = true;
    
    Serial.println("[Perf] Performance Manager initialized");
    Serial.printf("  Tick interval: %d us\n", _tickInterval);
    Serial.printf("  Running on core: %d\n", xPortGetCoreID());
    
    return true;
}

void PerformanceManager::end() {
    if (!_initialized) {
        return;
    }
    
    stopLvglTask();
    destroyTickTimer();
    _initialized = false;
}

void PerformanceManager::createTickTimer() {
    const esp_timer_create_args_t timerArgs = {
        .callback = tickTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "lvgl_tick"
    };
    
    esp_err_t err = esp_timer_create(&timerArgs, &_tickTimer);
    if (err != ESP_OK) {
        Serial.println("[Perf] Failed to create tick timer!");
        return;
    }
    
    esp_timer_start_periodic(_tickTimer, _tickInterval);
    Serial.println("[Perf] Tick timer started (1ms period)");
}

void PerformanceManager::destroyTickTimer() {
    if (_tickTimer) {
        esp_timer_stop(_tickTimer);
        esp_timer_delete(_tickTimer);
        _tickTimer = nullptr;
    }
}

void IRAM_ATTR PerformanceManager::tickTimerCallback(void* arg) {
    tickCounter++;
}

void PerformanceManager::lvglTaskEntry(void* arg) {
    PerformanceManager* perf = (PerformanceManager*)arg;
    
    Serial.println("[Perf] LVGL task started on Core 1");
    
    lvglTaskRunning = true;
    
    while (lvglTaskRunning) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    Serial.println("[Perf] LVGL task stopped");
    vTaskDelete(NULL);
}

void PerformanceManager::startLvglTask() {
    if (_lvglTaskHandle != nullptr) {
        Serial.println("[Perf] LVGL task already running");
        return;
    }
    
    lvglTaskRunning = true;
    
    BaseType_t ret = xTaskCreatePinnedToCore(
        lvglTaskEntry,
        "LVGL_Task",
        PERF_TASK_STACK_SIZE,
        this,
        PERF_TASK_PRIORITY,
        &_lvglTaskHandle,
        1
    );
    
    if (ret != pdPASS) {
        Serial.println("[Perf] Failed to create LVGL task!");
        _lvglTaskHandle = nullptr;
    } else {
        Serial.println("[Perf] LVGL task created on Core 1");
    }
}

void PerformanceManager::stopLvglTask() {
    if (_lvglTaskHandle == nullptr) {
        return;
    }
    
    lvglTaskRunning = false;
    vTaskDelay(pdMS_TO_TICKS(50));
    _lvglTaskHandle = nullptr;
}

void PerformanceManager::setMode(perf_mode_t mode) {
    _currentMode = mode;
    
    switch (mode) {
        case PERF_MODE_HIGH:
            setCpuFrequencyMhz(240);
            Serial.println("[Perf] Mode: HIGH (240MHz)");
            break;
        case PERF_MODE_BALANCED:
            setCpuFrequencyMhz(160);
            Serial.println("[Perf] Mode: BALANCED (160MHz)");
            break;
        case PERF_MODE_LOW:
            setCpuFrequencyMhz(80);
            Serial.println("[Perf] Mode: LOW (80MHz)");
            break;
    }
}

void PerformanceManager::updateStats() {
    _stats.tickCount = tickCounter;
    _stats.freeHeap = ESP.getFreeHeap();
    _stats.currentMode = _currentMode;
    
#if LV_USE_MEM_MONITOR
    lv_mem_monitor_t mem_mon;
    lv_mem_monitor(&mem_mon);
    _stats.lvglMemUsed = mem_mon.total_size - mem_mon.free_size;
    _stats.lvglMemFree = mem_mon.free_size;
    _stats.cpuUsage = mem_mon.used_pct;
#endif

    _stats.fps = 60;
}

perf_stats_t PerformanceManager::getStats() {
    updateStats();
    return _stats;
}

void PerformanceManager::printStats() {
    updateStats();
    
    Serial.println("\n[Perf] Performance Statistics");
    Serial.println("------------------------------");
    Serial.printf("  CPU Freq:    %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("  Free Heap:   %u bytes\n", _stats.freeHeap);
    Serial.printf("  Tick Count:  %u\n", _stats.tickCount);
    
#if LV_USE_MEM_MONITOR
    Serial.printf("  LVGL Used:   %u bytes\n", _stats.lvglMemUsed);
    Serial.printf("  LVGL Free:   %u bytes\n", _stats.lvglMemFree);
    Serial.printf("  LVGL Usage:  %d%%\n", _stats.cpuUsage);
#endif

#if LV_USE_PERF_MONITOR
    Serial.printf("  FPS:         %u\n", _stats.fps);
#endif
    
    Serial.println("------------------------------\n");
}

void PerformanceManager::setTickInterval(uint32_t intervalUs) {
    _tickInterval = intervalUs;
    
    if (_tickTimer && _initialized) {
        esp_timer_stop(_tickTimer);
        esp_timer_start_periodic(_tickTimer, _tickInterval);
    }
}
