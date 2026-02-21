#include "Storage.h"
#include "ConfigManager.h"
#include <ArduinoJson.h>

static lv_fs_drv_t spiffs_drv;
static lv_fs_drv_t sd_drv;

StorageManager Storage;

static void* spiffs_open_cb(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode) {
    String fullPath = String("/") + path;
    fs::File* file = new fs::File(SPIFFS.open(fullPath, "r"));
    if (!*file) {
        delete file;
        return NULL;
    }
    return file;
}

static lv_fs_res_t spiffs_close_cb(lv_fs_drv_t* drv, void* file_p) {
    fs::File* file = (fs::File*)file_p;
    if (file) {
        file->close();
        delete file;
    }
    return LV_FS_RES_OK;
}

static lv_fs_res_t spiffs_read_cb(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br) {
    fs::File* file = (fs::File*)file_p;
    if (!file || !*file) {
        *br = 0;
        return LV_FS_RES_FS_ERR;
    }
    *br = file->read((uint8_t*)buf, btr);
    return LV_FS_RES_OK;
}

static lv_fs_res_t spiffs_seek_cb(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t whence) {
    fs::File* file = (fs::File*)file_p;
    if (!file || !*file) return LV_FS_RES_FS_ERR;
    
    SeekMode mode = SeekSet;
    if (whence == LV_FS_SEEK_CUR) mode = SeekCur;
    else if (whence == LV_FS_SEEK_END) mode = SeekEnd;
    
    file->seek(pos, mode);
    return LV_FS_RES_OK;
}

static lv_fs_res_t spiffs_tell_cb(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p) {
    fs::File* file = (fs::File*)file_p;
    if (!file || !*file) return LV_FS_RES_FS_ERR;
    *pos_p = file->position();
    return LV_FS_RES_OK;
}

static void* sd_open_cb(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode) {
    String fullPath = String("/") + path;
    fs::File* file = new fs::File(SD.open(fullPath, "r"));
    if (!*file) {
        delete file;
        return NULL;
    }
    return file;
}

static lv_fs_res_t sd_close_cb(lv_fs_drv_t* drv, void* file_p) {
    fs::File* file = (fs::File*)file_p;
    if (file) {
        file->close();
        delete file;
    }
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_read_cb(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br) {
    fs::File* file = (fs::File*)file_p;
    if (!file || !*file) {
        *br = 0;
        return LV_FS_RES_FS_ERR;
    }
    *br = file->read((uint8_t*)buf, btr);
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_seek_cb(lv_fs_drv_t* drv, void* file_p, uint32_t pos, lv_fs_whence_t whence) {
    fs::File* file = (fs::File*)file_p;
    if (!file || !*file) return LV_FS_RES_FS_ERR;
    
    SeekMode mode = SeekSet;
    if (whence == LV_FS_SEEK_CUR) mode = SeekCur;
    else if (whence == LV_FS_SEEK_END) mode = SeekEnd;
    
    file->seek(pos, mode);
    return LV_FS_RES_OK;
}

static lv_fs_res_t sd_tell_cb(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p) {
    fs::File* file = (fs::File*)file_p;
    if (!file || !*file) return LV_FS_RES_FS_ERR;
    *pos_p = file->position();
    return LV_FS_RES_OK;
}

void storage_fs_init(void) {
    lv_fs_drv_init(&spiffs_drv);
    spiffs_drv.letter = STORAGE_DRIVE_SPIFFS;
    spiffs_drv.open_cb = spiffs_open_cb;
    spiffs_drv.close_cb = spiffs_close_cb;
    spiffs_drv.read_cb = spiffs_read_cb;
    spiffs_drv.seek_cb = spiffs_seek_cb;
    spiffs_drv.tell_cb = spiffs_tell_cb;
    lv_fs_drv_register(&spiffs_drv);
    
    lv_fs_drv_init(&sd_drv);
    sd_drv.letter = STORAGE_DRIVE_SD;
    sd_drv.open_cb = sd_open_cb;
    sd_drv.close_cb = sd_close_cb;
    sd_drv.read_cb = sd_read_cb;
    sd_drv.seek_cb = sd_seek_cb;
    sd_drv.tell_cb = sd_tell_cb;
    lv_fs_drv_register(&sd_drv);
}

StorageManager::StorageManager() {
    spiffsReady = false;
    sdReady = false;
    cacheUsedMemory = 0;
    accessCounter = 0;
    initCache();
}

StorageManager::~StorageManager() {
    clearCache();
}

void StorageManager::initCache() {
    for (int i = 0; i < CACHE_MAX_ENTRIES; i++) {
        cache[i].path[0] = '\0';
        cache[i].data = NULL;
        cache[i].size = 0;
        cache[i].lastAccess = 0;
        cache[i].valid = false;
    }
    cacheUsedMemory = 0;
}

bool StorageManager::begin() {
    Serial.println("\n[Storage] Initializing...");
    
    initSPIFFS();
    initSD();
    
    storage_fs_init();
    
    Serial.println("[Storage] LVGL FS drivers registered");
    Serial.printf("  Drive F: (SPIFFS) %s\n", spiffsReady ? "READY" : "NOT READY");
    Serial.printf("  Drive S: (SD Card) %s\n", sdReady ? "READY" : "NOT READY");
    
    return spiffsReady || sdReady;
}

bool StorageManager::initSPIFFS() {
    Serial.println("[Storage] Mounting SPIFFS...");
    
    if (!SPIFFS.begin(true)) {
        Serial.println("  SPIFFS mount: FAILED, formatting...");
        if (!SPIFFS.format()) {
            Serial.println("  SPIFFS format: FAILED");
            spiffsReady = false;
            return false;
        }
        Serial.println("  SPIFFS format: OK");
        if (!SPIFFS.begin(true)) {
            Serial.println("  SPIFFS mount after format: FAILED");
            spiffsReady = false;
            return false;
        }
    }
    
    spiffsReady = true;
    
    Serial.printf("  SPIFFS: %d KB total\n", SPIFFS.totalBytes() / 1024);
    Serial.println("  SPIFFS mount: OK");
    return true;
}

bool StorageManager::initSD() {
    Serial.println("[Storage] Checking SD card...");
    
    if (!SD.begin()) {
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
    
    sdReady = true;
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("  SD card: %llu MB\n", cardSize);
    Serial.println("  SD card: OK");
    return true;
}

int StorageManager::findCacheEntry(const char* path) {
    for (int i = 0; i < CACHE_MAX_ENTRIES; i++) {
        if (cache[i].valid && strcmp(cache[i].path, path) == 0) {
            return i;
        }
    }
    return -1;
}

int StorageManager::findLRUEntry() {
    int lruIndex = -1;
    uint32_t oldestAccess = UINT32_MAX;
    
    for (int i = 0; i < CACHE_MAX_ENTRIES; i++) {
        if (cache[i].valid && cache[i].lastAccess < oldestAccess) {
            oldestAccess = cache[i].lastAccess;
            lruIndex = i;
        }
    }
    return lruIndex;
}

int StorageManager::findFreeEntry() {
    for (int i = 0; i < CACHE_MAX_ENTRIES; i++) {
        if (!cache[i].valid) {
            return i;
        }
    }
    return -1;
}

void StorageManager::evictEntry(int index) {
    if (index < 0 || index >= CACHE_MAX_ENTRIES) return;
    if (!cache[index].valid) return;
    
    if (cache[index].data) {
        cacheUsedMemory -= cache[index].size;
        free(cache[index].data);
        cache[index].data = NULL;
    }
    
    cache[index].path[0] = '\0';
    cache[index].size = 0;
    cache[index].lastAccess = 0;
    cache[index].valid = false;
}

uint8_t* StorageManager::loadFromCache(const char* path, size_t* size) {
    int index = findCacheEntry(path);
    
    if (index >= 0) {
        cache[index].lastAccess = ++accessCounter;
        *size = cache[index].size;
        return cache[index].data;
    }
    
    fs::File file = openFile(path);
    if (!file) {
        *size = 0;
        return NULL;
    }
    
    size_t fileSize = file.size();
    
    if (cacheUsedMemory + fileSize > CACHE_MAX_MEMORY) {
        while (cacheUsedMemory + fileSize > CACHE_MAX_MEMORY) {
            int lruIndex = findLRUEntry();
            if (lruIndex < 0) break;
            evictEntry(lruIndex);
        }
    }
    
    int freeIndex = findFreeEntry();
    if (freeIndex < 0) {
        int lruIndex = findLRUEntry();
        if (lruIndex >= 0) {
            evictEntry(lruIndex);
            freeIndex = findFreeEntry();
        }
    }
    
    if (freeIndex < 0) {
        file.close();
        *size = 0;
        return NULL;
    }
    
    uint8_t* buffer = (uint8_t*)malloc(fileSize);
    if (!buffer) {
        file.close();
        *size = 0;
        return NULL;
    }
    
    file.read(buffer, fileSize);
    file.close();
    
    strncpy(cache[freeIndex].path, path, sizeof(cache[freeIndex].path) - 1);
    cache[freeIndex].path[sizeof(cache[freeIndex].path) - 1] = '\0';
    cache[freeIndex].data = buffer;
    cache[freeIndex].size = fileSize;
    cache[freeIndex].lastAccess = ++accessCounter;
    cache[freeIndex].valid = true;
    cacheUsedMemory += fileSize;
    
    *size = fileSize;
    return buffer;
}

bool StorageManager::addToCache(const char* path, const uint8_t* data, size_t size) {
    if (findCacheEntry(path) >= 0) {
        return true;
    }
    
    if (cacheUsedMemory + size > CACHE_MAX_MEMORY) {
        while (cacheUsedMemory + size > CACHE_MAX_MEMORY) {
            int lruIndex = findLRUEntry();
            if (lruIndex < 0) break;
            evictEntry(lruIndex);
        }
    }
    
    int freeIndex = findFreeEntry();
    if (freeIndex < 0) {
        int lruIndex = findLRUEntry();
        if (lruIndex >= 0) {
            evictEntry(lruIndex);
            freeIndex = findFreeEntry();
        }
    }
    
    if (freeIndex < 0) return false;
    
    uint8_t* buffer = (uint8_t*)malloc(size);
    if (!buffer) return false;
    
    memcpy(buffer, data, size);
    
    strncpy(cache[freeIndex].path, path, sizeof(cache[freeIndex].path) - 1);
    cache[freeIndex].path[sizeof(cache[freeIndex].path) - 1] = '\0';
    cache[freeIndex].data = buffer;
    cache[freeIndex].size = size;
    cache[freeIndex].lastAccess = ++accessCounter;
    cache[freeIndex].valid = true;
    cacheUsedMemory += size;
    
    return true;
}

void StorageManager::clearCache() {
    for (int i = 0; i < CACHE_MAX_ENTRIES; i++) {
        if (cache[i].data) {
            free(cache[i].data);
            cache[i].data = NULL;
        }
        cache[i].valid = false;
    }
    cacheUsedMemory = 0;
}

void StorageManager::printCacheStatus() {
    Serial.println("\n[Storage] Cache Status");
    Serial.println("---------------------");
    
    int validCount = 0;
    for (int i = 0; i < CACHE_MAX_ENTRIES; i++) {
        if (cache[i].valid) {
            validCount++;
            Serial.printf("  [%d] %s: %d bytes\n", i, cache[i].path, cache[i].size);
        }
    }
    
    Serial.printf("  Entries: %d/%d\n", validCount, CACHE_MAX_ENTRIES);
    Serial.printf("  Memory: %d/%d bytes\n", cacheUsedMemory, CACHE_MAX_MEMORY);
    Serial.println("---------------------");
}

fs::File StorageManager::openFile(const char* path) {
    if (path[0] == STORAGE_DRIVE_SPIFFS && path[1] == ':') {
        if (!spiffsReady) return fs::File();
        return SPIFFS.open(path + 2, "r");
    }
    else if (path[0] == STORAGE_DRIVE_SD && path[1] == ':') {
        if (!sdReady) return fs::File();
        return SD.open(path + 2, "r");
    }
    
    if (spiffsReady) {
        fs::File f = SPIFFS.open(path, "r");
        if (f) return f;
    }
    
    if (sdReady) {
        return SD.open(path, "r");
    }
    
    return fs::File();
}

bool StorageManager::fileExists(const char* path) {
    if (path[0] == STORAGE_DRIVE_SPIFFS && path[1] == ':') {
        return spiffsReady && SPIFFS.exists(path + 2);
    }
    else if (path[0] == STORAGE_DRIVE_SD && path[1] == ':') {
        return sdReady && SD.exists(path + 2);
    }
    
    if (spiffsReady && SPIFFS.exists(path)) return true;
    if (sdReady && SD.exists(path)) return true;
    
    return false;
}

size_t StorageManager::getFileSize(const char* path) {
    fs::File file = openFile(path);
    if (!file) return 0;
    size_t size = file.size();
    file.close();
    return size;
}

void StorageManager::listSPIFFS(const char* path) {
    if (!spiffsReady) {
        Serial.println("SPIFFS not ready");
        return;
    }
    
    Serial.printf("\n[SPIFFS] Listing %s:\n", path);
    fs::File root = SPIFFS.open(path);
    if (!root || !root.isDirectory()) {
        Serial.println("  Failed to open directory");
        return;
    }
    
    fs::File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("  DIR: %s\n", file.name());
        } else {
            Serial.printf("  %s: %d bytes\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

void StorageManager::listSD(const char* path) {
    if (!sdReady) {
        Serial.println("SD card not ready");
        return;
    }
    
    Serial.printf("\n[SD] Listing %s:\n", path);
    fs::File root = SD.open(path);
    if (!root || !root.isDirectory()) {
        Serial.println("  Failed to open directory");
        return;
    }
    
    fs::File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("  DIR: %s\n", file.name());
        } else {
            Serial.printf("  %s: %d bytes\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

bool StorageManager::preloadResources(const char* manifestPath) {
    fs::File file = openFile(manifestPath);
    if (!file) {
        Serial.printf("[Storage] Manifest not found: %s\n", manifestPath);
        return false;
    }
    
    String jsonContent = file.readString();
    file.close();
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    
    if (error) {
        Serial.printf("[Storage] JSON parse error: %s\n", error.c_str());
        return false;
    }
    
    JsonObject resources = doc["resources"];
    
    if (resources["images"].is<JsonArray>()) {
        JsonArray images = resources["images"];
        for (JsonVariant img : images) {
            const char* imgPath = img.as<const char*>();
            size_t size;
            if (loadFromCache(imgPath, &size)) {
                Serial.printf("  Preloaded: %s (%d bytes)\n", imgPath, size);
            }
        }
    }
    
    if (resources["fonts"].is<JsonArray>()) {
        JsonArray fonts = resources["fonts"];
        for (JsonVariant font : fonts) {
            const char* fontPath = font.as<const char*>();
            size_t size;
            if (loadFromCache(fontPath, &size)) {
                Serial.printf("  Preloaded: %s (%d bytes)\n", fontPath, size);
            }
        }
    }
    
    return true;
}

void StorageManager::printStatus() {
    Serial.println("\n[Storage] Status Report");
    Serial.println("----------------------");
    Serial.printf("  SPIFFS:  %s\n", spiffsReady ? "READY" : "NOT READY");
    Serial.printf("  SD Card: %s\n", sdReady ? "READY" : "NOT READY");
    Serial.printf("  Cache:   %d/%d bytes used\n", cacheUsedMemory, CACHE_MAX_MEMORY);
    Serial.println("----------------------");
}
