#ifndef FILEEXPLORERAPP_H
#define FILEEXPLORERAPP_H

#include "AppManager.h"
#include <vector>

#define MAX_PATH_LENGTH 128
#define MAX_FILES_DISPLAY 12
#define FILENAME_MAX_LEN 32

typedef enum {
    STORAGE_SPIFFS = 0,
    STORAGE_SD = 1
} storage_type_t;

typedef enum {
    MODE_BROWSE = 0,
    MODE_SELECT_FILE = 1
} explorer_mode_t;

typedef struct {
    char name[FILENAME_MAX_LEN];
    bool isDirectory;
    size_t size;
} file_entry_t;

typedef void (*file_select_callback_t)(const char* path);

class FileExplorerApp : public BaseApp {
private:
    lv_obj_t* labelPath;
    lv_obj_t* labelStatus;
    lv_obj_t* listFiles;
    lv_obj_t* btnBack;
    lv_obj_t* btnUp;
    lv_obj_t* btnSwitch;
    lv_obj_t* btnSelect;
    
    char currentPath[MAX_PATH_LENGTH];
    storage_type_t currentStorage;
    
    std::vector<file_entry_t> fileList;
    int selectedIndex;
    
    bool sdCardAvailable;
    bool spiffsAvailable;
    
    uint32_t _lastUpdateMs;
    
    bool createUI() override;
    void destroyUI() override;
    
    void refreshFileList();
    void updatePathDisplay();
    void updateStatusDisplay();
    
    void navigateUp();
    void enterDirectory(const char* name);
    void selectFile(int index);
    
    void switchStorage();
    void confirmSelection();
    
    static void back_btn_cb(lv_event_t* e);
    static void up_btn_cb(lv_event_t* e);
    static void switch_btn_cb(lv_event_t* e);
    static void list_click_cb(lv_event_t* e);
    static void select_btn_cb(lv_event_t* e);
    
public:
    FileExplorerApp();
    ~FileExplorerApp();
    
    void onUpdate() override;
    app_info_t getInfo() const override;
    
    void setSelectMode(explorer_mode_t mode, const char* startPath = nullptr);
    static explorer_mode_t getMode() { return _explorerMode; }
    
    static file_select_callback_t selectCallback;
    static char selectStartPath[MAX_PATH_LENGTH];
    static explorer_mode_t _explorerMode;
};

BaseApp* createFileExplorerApp();

#endif
