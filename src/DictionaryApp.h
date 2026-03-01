#ifndef DICTIONARYAPP_H
#define DICTIONARYAPP_H

#include "AppManager.h"
#include <vector>
#include <SD.h>

#define DICT_WORD_MAX_LEN       64
#define DICT_PHONETIC_MAX_LEN  32
#define DICT_TRANS_MAX_LEN      256
#define DICT_DEF_MAX_LEN       512
#define DICT_POS_MAX_LEN       16
#define DICT_INPUT_MAX_LEN     64
#define DICT_MAX_RESULTS       20
#define DICT_MAX_HISTORY      10
#define DICT_CACHE_PATH        "/Dictionary/.cache"

typedef enum {
    DICT_PAGE_SEARCH = 0,
    DICT_PAGE_RESULT = 1,
    DICT_PAGE_DETAIL = 2
} dict_page_t;

typedef struct {
    char word[DICT_WORD_MAX_LEN];
    char phonetic[DICT_PHONETIC_MAX_LEN];
    char translation[DICT_TRANS_MAX_LEN];
    char definition[DICT_DEF_MAX_LEN];
    char pos[DICT_POS_MAX_LEN];
} dict_entry_t;

typedef struct {
    char word[DICT_WORD_MAX_LEN];
    int frequency;
} hot_word_t;

class DictionaryApp : public BaseApp {
private:
    lv_obj_t* searchPage;
    lv_obj_t* resultPage;
    lv_obj_t* detailPage;
    
    lv_obj_t* searchInput;
    lv_obj_t* historyContainer;
    lv_obj_t* hotWordsContainer;
    
    lv_obj_t* resultList;
    lv_obj_t* resultBackBtn;
    lv_obj_t* resultLabel;
    
    lv_obj_t* detailPanel;
    lv_obj_t* detailBackBtn;
    lv_obj_t* detailWord;
    lv_obj_t* detailPhonetic;
    lv_obj_t* detailPos;
    lv_obj_t* detailTranslation;
    lv_obj_t* detailDefinition;
    
    lv_obj_t* keyboard;
    
    dict_page_t currentPage;
    
    std::vector<dict_entry_t> searchResults;
    std::vector<String> searchHistory;
    std::vector<hot_word_t> hotWords;
    
    char lastSearch[DICT_INPUT_MAX_LEN];
    int selectedResultIndex;
    
    bool sdCardAvailable;
    bool dictLoaded;
    
    File dictFile;
    uint32_t dictFileSize;
    
    lv_timer_t* searchTimer;
    bool searchPending;
    
    bool createUI() override;
    void destroyUI() override;
    bool onResume() override;
    void onPause() override;
    
    void createSearchPage();
    void createResultPage();
    void createDetailPage();
    
    void showSearchPage();
    void showResultPage();
    void showDetailPage(int index);
    
    void loadDictionaryIndex();
    bool searchDictionary(const char* keyword);
    void performSearch();
    
    void displayResults();
    void displayDetail(int index);
    
    void updateHistoryButtons();
    void updateHotWordsButtons();
    
    void saveState() override;
    bool loadState() override;
    
    void clearSearchResults();
    void clearPages();
    
    static void search_input_cb(lv_event_t* e);
    static void search_timer_cb(lv_timer_t* timer);
    static void keyboard_event_cb(lv_event_t* e);
    static void history_btn_cb(lv_event_t* e);
    static void hot_word_btn_cb(lv_event_t* e);
    static void result_list_cb(lv_event_t* e);
    static void result_back_cb(lv_event_t* e);
    static void detail_back_cb(lv_event_t* e);
    
    void onSearchInput();
    void onHistoryButtonClick(const char* word);
    void onHotWordButtonClick(const char* word);
    void onResultItemClick(int index);
    void onResultBack();
    void onDetailBack();
    
    void addToHistory(const char* word);
    void updateHotWordFrequency(const char* word);
    
public:
    DictionaryApp();
    ~DictionaryApp();
    
    void onUpdate() override;
    app_info_t getInfo() const override;
};

BaseApp* createDictionaryApp();

#endif
