#include "FileExplorerApp.h"
#include "AppManager.h"
#include "Storage.h"
#include "BSP.h"
#include <SD.h>
#include <SPIFFS.h>

FileExplorerApp::FileExplorerApp() : BaseApp("FileExplorer") {
    labelPath = nullptr;
    labelStatus = nullptr;
    listFiles = nullptr;
    btnBack = nullptr;
    btnUp = nullptr;
    btnSwitch = nullptr;
    
    strcpy(currentPath, "/");
    currentStorage = STORAGE_SD;
    selectedIndex = -1;
    sdCardAvailable = false;
    spiffsAvailable = false;
    _lastUpdateMs = 0;
}

FileExplorerApp::~FileExplorerApp() {
    fileList.clear();
}

bool FileExplorerApp::createUI() {
    lv_obj_t* scr = getScreen();
    
    sdCardAvailable = Storage.isSDReady();
    spiffsAvailable = Storage.isSPIFFSReady();
    
    if (!sdCardAvailable && !spiffsAvailable) {
        lv_obj_t* labelError = lv_label_create(scr);
        lv_label_set_text(labelError, "No storage available!\nPlease insert SD card.");
        lv_obj_set_style_text_color(labelError, lv_color_make(0xFF, 0x00, 0x00), 0);
        lv_obj_set_style_text_font(labelError, &lv_font_montserrat_14, 0);
        lv_obj_center(labelError);
        
        btnBack = lv_btn_create(scr);
        lv_obj_set_size(btnBack, 80, 35);
        lv_obj_align(btnBack, LV_ALIGN_BOTTOM_LEFT, 10, -10);
        lv_obj_add_event_cb(btnBack, back_btn_cb, LV_EVENT_CLICKED, this);
        lv_obj_set_style_bg_color(btnBack, lv_color_make(0x40, 0x40, 0x80), 0);
        
        lv_obj_t* btnLabel = lv_label_create(btnBack);
        lv_label_set_text(btnLabel, LV_SYMBOL_LEFT " Back");
        lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_14, 0);
        lv_obj_center(btnLabel);
        
        return true;
    }
    
    if (sdCardAvailable) {
        currentStorage = STORAGE_SD;
    } else {
        currentStorage = STORAGE_SPIFFS;
    }
    
    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, LV_SYMBOL_DIRECTORY " File Explorer");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    labelPath = lv_label_create(scr);
    lv_label_set_text(labelPath, "S:/");
    lv_obj_set_style_text_color(labelPath, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelPath, &lv_font_montserrat_10, 0);
    lv_obj_set_width(labelPath, BSP_DISPLAY_WIDTH - 20);
    lv_label_set_long_mode(labelPath, LV_LABEL_LONG_SCROLL);
    lv_obj_align(labelPath, LV_ALIGN_TOP_LEFT, 10, 28);
    
    listFiles = lv_list_create(scr);
    lv_obj_set_size(listFiles, BSP_DISPLAY_WIDTH - 20, BSP_DISPLAY_HEIGHT - 110);
    lv_obj_align(listFiles, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_style_bg_color(listFiles, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(listFiles, 1, 0);
    lv_obj_set_style_border_color(listFiles, lv_color_make(0x40, 0x40, 0x40), 0);
    
    labelStatus = lv_label_create(scr);
    lv_label_set_text(labelStatus, "SD: -- | SPIFFS: --");
    lv_obj_set_style_text_color(labelStatus, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_text_font(labelStatus, &lv_font_montserrat_10, 0);
    lv_obj_align(labelStatus, LV_ALIGN_BOTTOM_LEFT, 10, -45);
    
    btnSwitch = lv_btn_create(scr);
    lv_obj_set_size(btnSwitch, 70, 30);
    lv_obj_align(btnSwitch, LV_ALIGN_BOTTOM_RIGHT, -95, -48);
    lv_obj_add_event_cb(btnSwitch, switch_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnSwitch, lv_color_make(0x00, 0x60, 0x40), 0);
    
    lv_obj_t* switchLabel = lv_label_create(btnSwitch);
    lv_label_set_text(switchLabel, "Switch");
    lv_obj_set_style_text_font(switchLabel, &lv_font_montserrat_10, 0);
    lv_obj_center(switchLabel);
    
    btnUp = lv_btn_create(scr);
    lv_obj_set_size(btnUp, 70, 30);
    lv_obj_align(btnUp, LV_ALIGN_BOTTOM_RIGHT, -10, -48);
    lv_obj_add_event_cb(btnUp, up_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnUp, lv_color_make(0x60, 0x40, 0x00), 0);
    
    lv_obj_t* upLabel = lv_label_create(btnUp);
    lv_label_set_text(upLabel, LV_SYMBOL_UP " Up");
    lv_obj_set_style_text_font(upLabel, &lv_font_montserrat_10, 0);
    lv_obj_center(upLabel);
    
    btnBack = lv_btn_create(scr);
    lv_obj_set_size(btnBack, 80, 30);
    lv_obj_align(btnBack, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_add_event_cb(btnBack, back_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnBack, lv_color_make(0x40, 0x40, 0x80), 0);
    
    lv_obj_t* btnLabel = lv_label_create(btnBack);
    lv_label_set_text(btnLabel, LV_SYMBOL_LEFT " Back");
    lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(btnLabel);
    
    refreshFileList();
    
    return true;
}

void FileExplorerApp::destroyUI() {
    fileList.clear();
}

void FileExplorerApp::refreshFileList() {
    if (!listFiles) return;
    
    fileList.clear();
    
    lv_obj_clean(listFiles);
    
    fs::FS* fs = nullptr;
    char driveLetter = 'S';
    
    if (currentStorage == STORAGE_SD) {
        if (!sdCardAvailable) {
            lv_list_add_btn(listFiles, LV_SYMBOL_WARNING, "SD not available");
            return;
        }
        fs = &SD;
        driveLetter = 'S';
    } else {
        if (!spiffsAvailable) {
            lv_list_add_btn(listFiles, LV_SYMBOL_WARNING, "SPIFFS not available");
            return;
        }
        fs = &SPIFFS;
        driveLetter = 'F';
    }
    
    File root = fs->open(currentPath);
    if (!root || !root.isDirectory()) {
        lv_list_add_btn(listFiles, LV_SYMBOL_WARNING, "Cannot open directory");
        return;
    }
    
    File file = root.openNextFile();
    int count = 0;
    
    while (file && count < MAX_FILES_DISPLAY) {
        file_entry_t entry;
        
        const char* rawName = file.name();
        const char* displayName = rawName;
        
        const char* lastSlash = strrchr(rawName, '/');
        if (lastSlash) {
            displayName = lastSlash + 1;
        }
        
        strncpy(entry.name, displayName, FILENAME_MAX_LEN - 1);
        entry.name[FILENAME_MAX_LEN - 1] = '\0';
        entry.isDirectory = file.isDirectory();
        entry.size = file.size();
        
        fileList.push_back(entry);
        
        const char* icon = entry.isDirectory ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_FILE;
        char displayNameBuf[FILENAME_MAX_LEN + 16];
        
        if (entry.isDirectory) {
            snprintf(displayNameBuf, sizeof(displayNameBuf), "[%s]", entry.name);
        } else {
            if (entry.size < 1024) {
                snprintf(displayNameBuf, sizeof(displayNameBuf), "%s (%d B)", entry.name, (int)entry.size);
            } else if (entry.size < 1024 * 1024) {
                snprintf(displayNameBuf, sizeof(displayNameBuf), "%s (%d KB)", entry.name, (int)(entry.size / 1024));
            } else {
                snprintf(displayNameBuf, sizeof(displayNameBuf), "%s (%d MB)", entry.name, (int)(entry.size / (1024 * 1024)));
            }
        }
        
        lv_obj_t* btn = lv_list_add_btn(listFiles, icon, displayNameBuf);
        lv_obj_add_event_cb(btn, list_click_cb, LV_EVENT_CLICKED, this);
        
        file = root.openNextFile();
        count++;
    }
    
    root.close();
    
    updatePathDisplay();
    updateStatusDisplay();
    
    Serial.printf("[FileExplorer] Listed %d items in %c:%s\n", count, driveLetter, currentPath);
}

void FileExplorerApp::updatePathDisplay() {
    if (!labelPath) return;
    
    char displayPath[MAX_PATH_LENGTH + 4];
    char driveLetter = (currentStorage == STORAGE_SD) ? 'S' : 'F';
    
    snprintf(displayPath, sizeof(displayPath), "%c:%s", driveLetter, currentPath);
    lv_label_set_text(labelPath, displayPath);
}

void FileExplorerApp::updateStatusDisplay() {
    if (!labelStatus) return;
    
    char status[64];
    
    const char* sdStatus = sdCardAvailable ? "OK" : "--";
    const char* spiffsStatus = spiffsAvailable ? "OK" : "--";
    const char* currentStr = (currentStorage == STORAGE_SD) ? "[SD]" : "[SPIFFS]";
    
    snprintf(status, sizeof(status), "SD:%s SPIFFS:%s %s", sdStatus, spiffsStatus, currentStr);
    lv_label_set_text(labelStatus, status);
}

void FileExplorerApp::navigateUp() {
    if (strcmp(currentPath, "/") == 0) {
        return;
    }
    
    char* lastSlash = strrchr(currentPath, '/');
    if (lastSlash == currentPath) {
        strcpy(currentPath, "/");
    } else if (lastSlash) {
        *lastSlash = '\0';
    }
    
    refreshFileList();
}

void FileExplorerApp::enterDirectory(const char* name) {
    if (strcmp(currentPath, "/") == 0) {
        snprintf(currentPath, MAX_PATH_LENGTH, "/%s", name);
    } else {
        size_t currentLen = strlen(currentPath);
        if (currentLen + strlen(name) + 2 < MAX_PATH_LENGTH) {
            strcat(currentPath, "/");
            strcat(currentPath, name);
        }
    }
    
    refreshFileList();
}

void FileExplorerApp::selectFile(int index) {
    if (index < 0 || index >= (int)fileList.size()) {
        return;
    }
    
    file_entry_t& entry = fileList[index];
    
    if (entry.isDirectory) {
        enterDirectory(entry.name);
    } else {
        Serial.printf("[FileExplorer] Selected file: %s (%d bytes)\n", entry.name, (int)entry.size);
    }
}

void FileExplorerApp::switchStorage() {
    if (currentStorage == STORAGE_SD && spiffsAvailable) {
        currentStorage = STORAGE_SPIFFS;
        strcpy(currentPath, "/");
        refreshFileList();
    } else if (currentStorage == STORAGE_SPIFFS && sdCardAvailable) {
        currentStorage = STORAGE_SD;
        strcpy(currentPath, "/");
        refreshFileList();
    }
}

void FileExplorerApp::back_btn_cb(lv_event_t* e) {
    AppMgr.switchToHome();
}

void FileExplorerApp::up_btn_cb(lv_event_t* e) {
    FileExplorerApp* app = (FileExplorerApp*)lv_event_get_user_data(e);
    app->navigateUp();
}

void FileExplorerApp::switch_btn_cb(lv_event_t* e) {
    FileExplorerApp* app = (FileExplorerApp*)lv_event_get_user_data(e);
    app->switchStorage();
}

void FileExplorerApp::list_click_cb(lv_event_t* e) {
    FileExplorerApp* app = (FileExplorerApp*)lv_event_get_user_data(e);
    lv_obj_t* btn = lv_event_get_target(e);
    
    lv_obj_t* list = lv_obj_get_parent(btn);
    int index = 0;
    
    lv_obj_t* child = lv_obj_get_child(list, 0);
    while (child && child != btn) {
        child = lv_obj_get_child(list, ++index);
    }
    
    if (child == btn) {
        app->selectFile(index);
    }
}

void FileExplorerApp::onUpdate() {
    uint32_t now = millis();
    
    if (now - _lastUpdateMs < 2000) {
        return;
    }
    _lastUpdateMs = now;
    
    bool sdNow = Storage.isSDReady();
    bool spiffsNow = Storage.isSPIFFSReady();
    
    if (sdNow != sdCardAvailable || spiffsNow != spiffsAvailable) {
        sdCardAvailable = sdNow;
        spiffsAvailable = spiffsNow;
        
        if (!sdCardAvailable && !spiffsAvailable) {
            return;
        }
        
        if (currentStorage == STORAGE_SD && !sdCardAvailable && spiffsAvailable) {
            currentStorage = STORAGE_SPIFFS;
            strcpy(currentPath, "/");
        } else if (currentStorage == STORAGE_SPIFFS && !spiffsAvailable && sdCardAvailable) {
            currentStorage = STORAGE_SD;
            strcpy(currentPath, "/");
        }
        
        refreshFileList();
    }
}

app_info_t FileExplorerApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "Files", APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, LV_SYMBOL_DIRECTORY);
    info.type = APP_TYPE_SYSTEM;
    info.enabled = true;
    return info;
}

BaseApp* createFileExplorerApp() {
    return new FileExplorerApp();
}
