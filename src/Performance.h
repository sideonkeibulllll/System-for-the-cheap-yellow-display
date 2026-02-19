#ifndef PERFORMANCE_H
#define PERFORMANCE_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_timer.h>

#define PERF_TASK_STACK_SIZE    4096
#define PERF_TASK_PRIORITY      5
#define PERF_TICK_INTERVAL_US   1000

typedef struct {
    uint32_t fps;
    uint32_t freeHeap;
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
    
    perf_stats_t getStats();
    
    uint32_t getFreeHeap() { return _stats.freeHeap; }
    
    static void lvglTaskEntry(void* arg);
    static void tickTimerCallback(void* arg);
    
private:
    bool _initialized;
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
