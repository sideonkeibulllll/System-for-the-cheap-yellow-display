#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>
#include <lvgl.h>
#include <FS.h>
#include <SPIFFS.h>
#include <SD.h>

#define STORAGE_DRIVE_SPIFFS    'F'
#define STORAGE_DRIVE_SD        'S'

#define CACHE_MAX_ENTRIES       16
#define CACHE_MAX_MEMORY        (32 * 1024)

typedef struct {
    char path[128];
    uint8_t* data;
    size_t size;
    uint32_t lastAccess;
    bool valid;
} CacheEntry;

typedef struct {
    char imagePath[64];
    char fontPath[64];
} ResourceItem;

typedef struct {
    char screenName[32];
    ResourceItem* images;
    int imageCount;
    ResourceItem* fonts;
    int fontCount;
} ScreenManifest;

class StorageManager {
private:
    bool spiffsReady;
    bool sdReady;
    
    CacheEntry cache[CACHE_MAX_ENTRIES];
    size_t cacheUsedMemory;
    uint32_t accessCounter;
    
    void initLVGLFileSystem();
    void initCache();
    
    int findCacheEntry(const char* path);
    int findLRUEntry();
    int findFreeEntry();
    void evictEntry(int index);
    
public:
    StorageManager();
    ~StorageManager();
    
    bool begin();
    
    bool initSPIFFS();
    bool initSD();
    
    bool isSPIFFSReady() { return spiffsReady; }
    bool isSDReady() { return sdReady; }
    
    uint8_t* loadFromCache(const char* path, size_t* size);
    bool addToCache(const char* path, const uint8_t* data, size_t size);
    void clearCache();
    void printCacheStatus();
    
    bool preloadResources(const char* manifestPath);
    bool parseManifest(const char* jsonContent, ScreenManifest* manifest);
    
    fs::File openFile(const char* path);
    bool fileExists(const char* path);
    size_t getFileSize(const char* path);
    
    void listSPIFFS(const char* path = "/");
    void listSD(const char* path = "/");
    
    void printStatus();
};

extern StorageManager Storage;

void storage_fs_init(void);

#endif
