#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>
#include <esp_pm.h>

#define PERF_TASK_STACK_SIZE    4096
#define PERF_TASK_PRIORITY      5
#define PERF_TICK_INTERVAL_US   1000

typedef enum {
    PERF_MODE_HIGH = 0,
    PERF_MODE_BALANCED = 1,
    PERF_MODE_LOW = 2
} perf_mode_t;

typedef struct {
    uint32_t fps;
    uint32_t cpuUsage;
    uint32_t freeHeap;
    uint32_t lvglMemUsed;
    uint32_t lvglMemFree;
    perf_mode_t currentMode;
    uint32_t tickCount;
} perf_stats_t;

class PerformanceManager {
public:
    PerformanceManager();
    ~PerformanceManager();
    
    bool begin();
    void end();
    
    void startLvglTask();
    void stopLvglTask();
    
    void setMode(perf_mode_t mode);
    perf_mode_t getMode() { return _currentMode; }
    
    perf_stats_t getStats();
    void printStats();
    
    uint32_t getFPS() { return _stats.fps; }
    uint32_t getFreeHeap() { return _stats.freeHeap; }
    
    void setTickInterval(uint32_t intervalUs);
    
    static void lvglTaskEntry(void* arg);
    static void tickTimerCallback(void* arg);
    
private:
    bool _initialized;
    perf_mode_t _currentMode;
    perf_stats_t _stats;
    
    TaskHandle_t _lvglTaskHandle;
    esp_timer_handle_t _tickTimer;
    uint32_t _tickInterval;
    
    void updateStats();
    void createTickTimer();
    void destroyTickTimer();
};

extern PerformanceManager Perf;

#endif
