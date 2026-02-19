#include "Performance.h"
#include "BSP.h"
#include <lvgl.h>

PerformanceManager Perf;

static volatile uint32_t tickCounter = 0;
static volatile bool lvglTaskRunning = false;

PerformanceManager::PerformanceManager() 
    : _initialized(false)
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
    
    createTickTimer();
    _initialized = true;
    
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
    
    esp_timer_create(&timerArgs, &_tickTimer);
    esp_timer_start_periodic(_tickTimer, _tickInterval);
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
    lvglTaskRunning = true;
    
    while (lvglTaskRunning) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    
    vTaskDelete(NULL);
}

void PerformanceManager::startLvglTask() {
    if (_lvglTaskHandle != nullptr) {
        return;
    }
    
    lvglTaskRunning = true;
    
    xTaskCreatePinnedToCore(
        lvglTaskEntry,
        "LVGL_Task",
        PERF_TASK_STACK_SIZE,
        this,
        PERF_TASK_PRIORITY,
        &_lvglTaskHandle,
        1
    );
}

void PerformanceManager::stopLvglTask() {
    if (_lvglTaskHandle == nullptr) {
        return;
    }
    
    lvglTaskRunning = false;
    vTaskDelay(pdMS_TO_TICKS(50));
    _lvglTaskHandle = nullptr;
}

void PerformanceManager::updateStats() {
    _stats.tickCount = tickCounter;
    _stats.freeHeap = ESP.getFreeHeap();
    _stats.fps = 0;
}

perf_stats_t PerformanceManager::getStats() {
    updateStats();
    return _stats;
}
