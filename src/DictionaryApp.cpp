#include "DictionaryApp.h"
#include "Storage.h"
#include "LvZhFont.h"
#include "BSP.h"
#include <SD.h>

DictionaryApp::DictionaryApp() : BaseApp("Dictionary") {
    searchPage = nullptr;
    resultPage = nullptr;
    detailPage = nullptr;
    
    searchInput = nullptr;
    historyContainer = nullptr;
    hotWordsContainer = nullptr;
    
    resultList = nullptr;
    resultBackBtn = nullptr;
    resultLabel = nullptr;
    
    detailPanel = nullptr;
    detailBackBtn = nullptr;
    detailWord = nullptr;
    detailPhonetic = nullptr;
    detailPos = nullptr;
    detailTranslation = nullptr;
    detailDefinition = nullptr;
    
    keyboard = nullptr;
    
    currentPage = DICT_PAGE_SEARCH;
    
    lastSearch[0] = '\0';
    selectedResultIndex = 0;
    
    sdCardAvailable = false;
    dictLoaded = false;
    
    dictFileSize = 0;
    
    searchTimer = nullptr;
    searchPending = false;
}

DictionaryApp::~DictionaryApp() {
    clearSearchResults();
    searchHistory.clear();
    hotWords.clear();
}

bool DictionaryApp::createUI() {
    Serial.println("[DictionaryApp] createUI start");
    
    sdCardAvailable = Storage.isSDReady();
    
    if (sdCardAvailable) {
        loadDictionaryIndex();
    }
    
    createSearchPage();
    createResultPage();
    createDetailPage();
    
    showSearchPage();
    
    Serial.println("[DictionaryApp] createUI done");
    return true;
}

void DictionaryApp::destroyUI() {
    Serial.println("[DictionaryApp] destroyUI");
    
    clearPages();
    
    if (searchTimer) {
        lv_timer_del(searchTimer);
        searchTimer = nullptr;
    }
    
    if (keyboard) {
        lv_obj_del(keyboard);
        keyboard = nullptr;
    }
    
    if (dictFile) {
        dictFile.close();
    }
    
    clearSearchResults();
}

void DictionaryApp::createSearchPage() {
    searchPage = lv_obj_create(_screen);
    lv_obj_set_size(searchPage, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(searchPage, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(searchPage, 0, 0);
    lv_obj_set_style_pad_all(searchPage, 10, 0);
    
    lv_obj_t* title = lv_label_create(searchPage);
    lv_label_set_text(title, "📖 词典搜索");
    lv_obj_set_style_text_font(title, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);
    
    searchInput = lv_textarea_create(searchPage);
    lv_obj_set_size(searchInput, 280, 50);
    lv_obj_align(searchInput, LV_ALIGN_TOP_MID, 0, 40);
    lv_textarea_set_max_length(searchInput, DICT_INPUT_MAX_LEN);
    lv_textarea_set_placeholder_text(searchInput, "Input...");
    lv_textarea_set_one_line(searchInput, true);
    lv_obj_set_style_bg_color(searchInput, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_set_style_bg_opa(searchInput, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(searchInput, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_font(searchInput, &lv_font_montserrat_14, 0);
    lv_obj_set_style_border_color(searchInput, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_border_width(searchInput, 2, 0);
    lv_obj_set_style_radius(searchInput, 6, 0);
    lv_obj_add_event_cb(searchInput, search_input_cb, LV_EVENT_FOCUSED, this);
    lv_obj_add_event_cb(searchInput, search_input_cb, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_add_event_cb(searchInput, search_input_cb, LV_EVENT_READY, this);
    
    lv_obj_t* historyLabel = lv_label_create(searchPage);
    lv_label_set_text(historyLabel, "最近搜索:");
    lv_obj_set_style_text_font(historyLabel, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(historyLabel, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_align(historyLabel, LV_ALIGN_TOP_MID, -100, 100);
    
    historyContainer = lv_obj_create(searchPage);
    lv_obj_set_size(historyContainer, 280, 40);
    lv_obj_align(historyContainer, LV_ALIGN_TOP_MID, 0, 125);
    lv_obj_set_style_bg_opa(historyContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(historyContainer, 0, 0);
    lv_obj_set_flex_flow(historyContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(historyContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(historyContainer, 5, 0);
    
    lv_obj_t* hotLabel = lv_label_create(searchPage);
    lv_label_set_text(hotLabel, "热门词汇:");
    lv_obj_set_style_text_font(hotLabel, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(hotLabel, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_align(hotLabel, LV_ALIGN_TOP_MID, -100, 175);
    
    hotWordsContainer = lv_obj_create(searchPage);
    lv_obj_set_size(hotWordsContainer, 280, 40);
    lv_obj_align(hotWordsContainer, LV_ALIGN_TOP_MID, 0, 200);
    lv_obj_set_style_bg_opa(hotWordsContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(hotWordsContainer, 0, 0);
    lv_obj_set_flex_flow(hotWordsContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hotWordsContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(hotWordsContainer, 5, 0);
    
    searchTimer = lv_timer_create(search_timer_cb, 300, this);
    lv_timer_pause(searchTimer);
}

void DictionaryApp::createResultPage() {
    resultPage = lv_obj_create(_screen);
    lv_obj_set_size(resultPage, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(resultPage, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(resultPage, 0, 0);
    lv_obj_set_style_pad_all(resultPage, 10, 0);
    
    resultBackBtn = lv_btn_create(resultPage);
    lv_obj_set_size(resultBackBtn, 60, 30);
    lv_obj_align(resultBackBtn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(resultBackBtn, lv_color_make(0x00, 0x80, 0xC0), 0);
    lv_obj_set_style_radius(resultBackBtn, 5, 0);
    lv_obj_add_event_cb(resultBackBtn, result_back_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* backLabel = lv_label_create(resultBackBtn);
    lv_label_set_text(backLabel, "← 返回");
    lv_obj_set_style_text_font(backLabel, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(backLabel, lv_color_white(), 0);
    lv_obj_center(backLabel);
    
    resultLabel = lv_label_create(resultPage);
    lv_label_set_text(resultLabel, "");
    lv_obj_set_style_text_font(resultLabel, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(resultLabel, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_align(resultLabel, LV_ALIGN_TOP_MID, 0, 40);
    
    resultList = lv_list_create(resultPage);
    lv_obj_set_size(resultList, 300, 240);
    lv_obj_align(resultList, LV_ALIGN_TOP_MID, 0, 65);
    lv_obj_set_style_bg_color(resultList, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_border_width(resultList, 0, 0);
    lv_obj_set_style_radius(resultList, 5, 0);
}

void DictionaryApp::createDetailPage() {
    detailPage = lv_obj_create(_screen);
    lv_obj_set_size(detailPage, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(detailPage, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(detailPage, 0, 0);
    lv_obj_set_style_pad_all(detailPage, 10, 0);
    
    detailBackBtn = lv_btn_create(detailPage);
    lv_obj_set_size(detailBackBtn, 60, 30);
    lv_obj_align(detailBackBtn, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_bg_color(detailBackBtn, lv_color_make(0x00, 0x80, 0xC0), 0);
    lv_obj_set_style_radius(detailBackBtn, 5, 0);
    lv_obj_add_event_cb(detailBackBtn, detail_back_cb, LV_EVENT_CLICKED, this);
    
    lv_obj_t* backLabel = lv_label_create(detailBackBtn);
    lv_label_set_text(backLabel, "← 返回");
    lv_obj_set_style_text_font(backLabel, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(backLabel, lv_color_white(), 0);
    lv_obj_center(backLabel);
    
    detailPanel = lv_obj_create(detailPage);
    lv_obj_set_size(detailPanel, 300, 280);
    lv_obj_align(detailPanel, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_bg_color(detailPanel, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_border_width(detailPanel, 0, 0);
    lv_obj_set_style_radius(detailPanel, 5, 0);
    lv_obj_set_flex_flow(detailPanel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(detailPanel, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(detailPanel, 10, 0);
    lv_obj_set_scroll_dir(detailPanel, LV_DIR_VER);
    
    detailWord = lv_label_create(detailPanel);
    lv_label_set_text(detailWord, "");
    lv_obj_set_style_text_font(detailWord, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(detailWord, lv_color_white(), 0);
    
    detailPhonetic = lv_label_create(detailPanel);
    lv_label_set_text(detailPhonetic, "");
    lv_obj_set_style_text_font(detailPhonetic, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(detailPhonetic, lv_color_make(0x80, 0xFF, 0x80), 0);
    
    detailPos = lv_label_create(detailPanel);
    lv_label_set_text(detailPos, "");
    lv_obj_set_style_text_font(detailPos, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(detailPos, lv_color_make(0x80, 0x80, 0xFF), 0);
    
    lv_obj_t* transLabel = lv_label_create(detailPanel);
    lv_label_set_text(transLabel, "中文释义:");
    lv_obj_set_style_text_font(transLabel, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(transLabel, lv_color_make(0xFF, 0x80, 0x80), 0);
    
    detailTranslation = lv_label_create(detailPanel);
    lv_label_set_text(detailTranslation, "");
    lv_obj_set_style_text_font(detailTranslation, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(detailTranslation, lv_color_white(), 0);
    lv_obj_set_style_text_align(detailTranslation, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(detailTranslation, 280);
    
    lv_obj_t* defLabel = lv_label_create(detailPanel);
    lv_label_set_text(defLabel, "英文释义:");
    lv_obj_set_style_text_font(defLabel, LvZhFontMgr.getFont(), 0);
    lv_obj_set_style_text_color(defLabel, lv_color_make(0xFF, 0x80, 0x80), 0);
    
    detailDefinition = lv_label_create(detailPanel);
    lv_label_set_text(detailDefinition, "");
    lv_obj_set_style_text_font(detailDefinition, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(detailDefinition, lv_color_make(0xC0, 0xC0, 0xC0), 0);
    lv_obj_set_style_text_align(detailDefinition, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_width(detailDefinition, 280);
}

void DictionaryApp::showSearchPage() {
    currentPage = DICT_PAGE_SEARCH;
    
    if (keyboard) {
        lv_obj_del(keyboard);
        keyboard = nullptr;
    }
    
    lv_obj_add_flag(searchPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(resultPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(detailPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(searchPage);
    
    updateHistoryButtons();
    updateHotWordsButtons();
}

void DictionaryApp::showResultPage() {
    currentPage = DICT_PAGE_RESULT;
    lv_obj_clear_flag(searchPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(resultPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(detailPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(resultPage);
}

void DictionaryApp::showDetailPage(int index) {
    selectedResultIndex = index;
    displayDetail(index);
    
    currentPage = DICT_PAGE_DETAIL;
    lv_obj_clear_flag(searchPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(resultPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(detailPage, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(detailPage);
}

void DictionaryApp::loadDictionaryIndex() {
    Serial.println("[DictionaryApp] Loading dictionary...");
    
    if (!SD.exists("/Dictionary/ecdict.csv")) {
        Serial.println("[DictionaryApp] Dictionary file not found");
        return;
    }
    
    dictFile = SD.open("/Dictionary/ecdict.csv");
    if (!dictFile) {
        Serial.println("[DictionaryApp] Failed to open dictionary");
        return;
    }
    
    dictFileSize = dictFile.size();
    dictLoaded = true;
    
    Serial.printf("[DictionaryApp] Dictionary loaded, size: %u bytes\n", dictFileSize);
}

bool DictionaryApp::searchDictionary(const char* keyword) {
    if (!dictLoaded || !dictFile) {
        Serial.println("[DictionaryApp] Dictionary not loaded");
        return false;
    }
    
    clearSearchResults();
    
    int len = strlen(keyword);
    if (len == 0) return false;
    
    Serial.printf("[DictionaryApp] Searching for: %s\n", keyword);
    
    char firstChar = tolower(keyword[0]);
    int matchCount = 0;
    int linesChecked = 0;
    int maxLinesToCheck = 50000;
    unsigned long startTime = millis();
    
    if (firstChar >= 'a' && firstChar <= 'z') {
        int letterIndex = firstChar - 'a';
        unsigned long startPos = (dictFileSize * letterIndex) / 30;
        unsigned long endPos = dictFileSize;
        
        Serial.printf("[DictionaryApp] Trying optimized search at byte %lu\n", startPos);
        
        dictFile.seek(startPos);
        if (startPos > 0) {
            dictFile.readStringUntil('\n');
        }
        
        while (dictFile.available() && matchCount < DICT_MAX_RESULTS && linesChecked < maxLinesToCheck) {
            unsigned long currentPos = dictFile.position();
            
            if (currentPos > endPos && matchCount > 0) {
                break;
            }
            
            String line = dictFile.readStringUntil('\n');
            linesChecked++;
            
            int commaPos = line.indexOf(',');
            if (commaPos <= 0) continue;
            
            String word = line.substring(0, commaPos);
            
            if (word.length() >= len && 
                strncasecmp(word.c_str(), keyword, len) == 0) {
                
                dict_entry_t entry;
                strncpy(entry.word, word.c_str(), DICT_WORD_MAX_LEN - 1);
                entry.word[DICT_WORD_MAX_LEN - 1] = '\0';
                entry.phonetic[0] = '\0';
                entry.translation[0] = '\0';
                entry.definition[0] = '\0';
                entry.pos[0] = '\0';
                
                int pos1 = commaPos + 1;
                int pos2 = line.indexOf(',', pos1);
                if (pos2 > pos1) {
                    String phonetic = line.substring(pos1, pos2);
                    strncpy(entry.phonetic, phonetic.c_str(), DICT_PHONETIC_MAX_LEN - 1);
                    entry.phonetic[DICT_PHONETIC_MAX_LEN - 1] = '\0';
                }
                
                pos1 = pos2 + 1;
                pos2 = line.indexOf(',', pos1);
                
                if (pos2 > pos1) {
                    String definition = line.substring(pos1, pos2);
                    strncpy(entry.definition, definition.c_str(), DICT_DEF_MAX_LEN - 1);
                    entry.definition[DICT_DEF_MAX_LEN - 1] = '\0';
                }
                
                pos1 = pos2 + 1;
                pos2 = line.indexOf(',', pos1);
                
                if (pos2 > pos1) {
                    String translation = line.substring(pos1, pos2);
                    strncpy(entry.translation, translation.c_str(), DICT_TRANS_MAX_LEN - 1);
                    entry.translation[DICT_TRANS_MAX_LEN - 1] = '\0';
                }
                
                pos1 = pos2 + 1;
                pos2 = line.indexOf(',', pos1);
                
                if (pos2 > pos1) {
                    String pos = line.substring(pos1, pos2);
                    strncpy(entry.pos, pos.c_str(), DICT_POS_MAX_LEN - 1);
                    entry.pos[DICT_POS_MAX_LEN - 1] = '\0';
                }
                
                searchResults.push_back(entry);
                matchCount++;
            }
        }
        
        if (matchCount > 0) {
            unsigned long searchTime = millis() - startTime;
            Serial.printf("[DictionaryApp] Found %d results (checked %d lines in %lu ms)\n", 
                          matchCount, linesChecked, searchTime);
            return true;
        }
        
        Serial.println("[DictionaryApp] Optimized search failed, trying from start...");
    }
    
    matchCount = 0;
    linesChecked = 0;
    maxLinesToCheck = 100000;
    
    dictFile.seek(0);
    
    while (dictFile.available() && matchCount < DICT_MAX_RESULTS && linesChecked < maxLinesToCheck) {
        String line = dictFile.readStringUntil('\n');
        linesChecked++;
        
        if (linesChecked % 10000 == 0) {
            Serial.printf("[DictionaryApp] Progress: %d lines checked\n", linesChecked);
        }
        
        int commaPos = line.indexOf(',');
        if (commaPos <= 0) continue;
        
        String word = line.substring(0, commaPos);
        
        if (word.length() >= len && 
            strncasecmp(word.c_str(), keyword, len) == 0) {
            
            dict_entry_t entry;
            strncpy(entry.word, word.c_str(), DICT_WORD_MAX_LEN - 1);
            entry.word[DICT_WORD_MAX_LEN - 1] = '\0';
            entry.phonetic[0] = '\0';
            entry.translation[0] = '\0';
            entry.definition[0] = '\0';
            entry.pos[0] = '\0';
            
            int pos1 = commaPos + 1;
            int pos2 = line.indexOf(',', pos1);
            if (pos2 > pos1) {
                String phonetic = line.substring(pos1, pos2);
                strncpy(entry.phonetic, phonetic.c_str(), DICT_PHONETIC_MAX_LEN - 1);
                entry.phonetic[DICT_PHONETIC_MAX_LEN - 1] = '\0';
            }
            
            pos1 = pos2 + 1;
            pos2 = line.indexOf(',', pos1);
            
            if (pos2 > pos1) {
                String definition = line.substring(pos1, pos2);
                strncpy(entry.definition, definition.c_str(), DICT_DEF_MAX_LEN - 1);
                entry.definition[DICT_DEF_MAX_LEN - 1] = '\0';
            }
            
            pos1 = pos2 + 1;
            pos2 = line.indexOf(',', pos1);
            
            if (pos2 > pos1) {
                String translation = line.substring(pos1, pos2);
                strncpy(entry.translation, translation.c_str(), DICT_TRANS_MAX_LEN - 1);
                entry.translation[DICT_TRANS_MAX_LEN - 1] = '\0';
            }
            
            pos1 = pos2 + 1;
            pos2 = line.indexOf(',', pos1);
            
            if (pos2 > pos1) {
                String pos = line.substring(pos1, pos2);
                strncpy(entry.pos, pos.c_str(), DICT_POS_MAX_LEN - 1);
                entry.pos[DICT_POS_MAX_LEN - 1] = '\0';
            }
            
            searchResults.push_back(entry);
            matchCount++;
        }
    }
    
    unsigned long searchTime = millis() - startTime;
    Serial.printf("[DictionaryApp] Found %d results (checked %d lines in %lu ms)\n", 
                  matchCount, linesChecked, searchTime);
    return matchCount > 0;
}

void DictionaryApp::performSearch() {
    const char* text = lv_textarea_get_text(searchInput);
    
    if (!text || strlen(text) == 0) {
        return;
    }
    
    strncpy(lastSearch, text, DICT_INPUT_MAX_LEN - 1);
    lastSearch[DICT_INPUT_MAX_LEN - 1] = '\0';
    
    Serial.printf("[DictionaryApp] Searching for: %s\n", lastSearch);
    
    if (searchDictionary(lastSearch)) {
        displayResults();
        showResultPage();
        addToHistory(lastSearch);
        updateHotWordFrequency(lastSearch);
    } else {
        Serial.println("[DictionaryApp] No results found");
    }
}

void DictionaryApp::displayResults() {
    lv_obj_clean(resultList);
    
    char buffer[256];
    
    for (size_t i = 0; i < searchResults.size(); i++) {
        const dict_entry_t& entry = searchResults[i];
        
        lv_obj_t* btn = lv_list_add_btn(resultList, LV_SYMBOL_FILE, NULL);
        lv_obj_set_style_text_font(btn, &lv_font_montserrat_14, 0);
        
        snprintf(buffer, sizeof(buffer), "%d. %s - %s", (int)i + 1, entry.word, entry.translation);
        lv_label_set_text(lv_obj_get_child(btn, 0), buffer);
        
        lv_obj_add_event_cb(btn, result_list_cb, LV_EVENT_CLICKED, this);
    }
    
    snprintf(buffer, sizeof(buffer), "搜索结果: \"%s\" (%zu 条)", lastSearch, searchResults.size());
    lv_label_set_text(resultLabel, buffer);
}

void DictionaryApp::displayDetail(int index) {
    if (index < 0 || index >= (int)searchResults.size()) {
        return;
    }
    
    const dict_entry_t& entry = searchResults[index];
    
    char buffer[512];
    
    lv_label_set_text(detailWord, entry.word);
    
    if (strlen(entry.phonetic) > 0) {
        snprintf(buffer, sizeof(buffer), "[%s]", entry.phonetic);
        lv_label_set_text(detailPhonetic, buffer);
    } else {
        lv_label_set_text(detailPhonetic, "");
    }
    
    if (strlen(entry.pos) > 0) {
        snprintf(buffer, sizeof(buffer), "词性: %s", entry.pos);
        lv_label_set_text(detailPos, buffer);
    } else {
        lv_label_set_text(detailPos, "");
    }
    
    lv_label_set_text(detailTranslation, entry.translation);
    lv_label_set_text(detailDefinition, entry.definition);
}

void DictionaryApp::updateHistoryButtons() {
    lv_obj_clean(historyContainer);
    
    for (const String& word : searchHistory) {
        lv_obj_t* btn = lv_btn_create(historyContainer);
        lv_obj_set_size(btn, 80, 30);
        lv_obj_set_style_bg_color(btn, lv_color_make(0x40, 0x40, 0x60), 0);
        lv_obj_set_style_radius(btn, 5, 0);
        lv_obj_add_event_cb(btn, history_btn_cb, LV_EVENT_CLICKED, this);
        
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, word.c_str());
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_center(label);
    }
}

void DictionaryApp::updateHotWordsButtons() {
    lv_obj_clean(hotWordsContainer);
    
    for (const hot_word_t& hot : hotWords) {
        lv_obj_t* btn = lv_btn_create(hotWordsContainer);
        lv_obj_set_size(btn, 80, 30);
        lv_obj_set_style_bg_color(btn, lv_color_make(0x60, 0x40, 0x60), 0);
        lv_obj_set_style_radius(btn, 5, 0);
        lv_obj_add_event_cb(btn, hot_word_btn_cb, LV_EVENT_CLICKED, this);
        
        lv_obj_t* label = lv_label_create(btn);
        lv_label_set_text(label, hot.word);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_center(label);
    }
}

void DictionaryApp::saveState() {
    if (!sdCardAvailable) return;
    
    File cacheFile = SD.open(DICT_CACHE_PATH, FILE_WRITE);
    if (!cacheFile) {
        Serial.println("[DictionaryApp] Failed to save cache");
        return;
    }
    
    cacheFile.printf("last_search=%s\n", lastSearch);
    cacheFile.printf("history_count=%d\n", (int)searchHistory.size());
    
    for (size_t i = 0; i < searchHistory.size(); i++) {
        cacheFile.printf("history_%d=%s\n", (int)i, searchHistory[i].c_str());
    }
    
    cacheFile.printf("hot_count=%d\n", (int)hotWords.size());
    
    for (size_t i = 0; i < hotWords.size(); i++) {
        cacheFile.printf("hot_%d=%s,%d\n", (int)i, hotWords[i].word, hotWords[i].frequency);
    }
    
    cacheFile.close();
    Serial.println("[DictionaryApp] Cache saved");
}

bool DictionaryApp::loadState() {
    if (!sdCardAvailable) return false;
    
    if (!SD.exists(DICT_CACHE_PATH)) {
        Serial.println("[DictionaryApp] No cache file found");
        return false;
    }
    
    File cacheFile = SD.open(DICT_CACHE_PATH);
    if (!cacheFile) {
        return false;
    }
    
    int historyCount = 0;
    int hotCount = 0;
    char savedLastSearch[DICT_INPUT_MAX_LEN] = "";
    
    while (cacheFile.available()) {
        String line = cacheFile.readStringUntil('\n');
        line.trim();
        
        if (line.startsWith("last_search=")) {
            strncpy(savedLastSearch, line.c_str() + 11, DICT_INPUT_MAX_LEN - 1);
            savedLastSearch[DICT_INPUT_MAX_LEN - 1] = '\0';
        } else if (line.startsWith("history_count=")) {
            historyCount = atoi(line.c_str() + 14);
        } else if (line.startsWith("history_")) {
            int idx = atoi(line.c_str() + 8);
            if (idx < historyCount) {
                int pos = line.indexOf('=');
                if (pos > 0) {
                    String word = line.substring(pos + 1);
                    searchHistory.push_back(word);
                }
            }
        } else if (line.startsWith("hot_count=")) {
            hotCount = atoi(line.c_str() + 10);
        } else if (line.startsWith("hot_")) {
            int idx = atoi(line.c_str() + 4);
            if (idx < hotCount) {
                int pos1 = line.indexOf('=');
                int pos2 = line.indexOf(',');
                if (pos1 > 0 && pos2 > pos1) {
                    String word = line.substring(pos1 + 1, pos2);
                    int freq = atoi(line.c_str() + pos2 + 1);
                    hot_word_t hot;
                    strncpy(hot.word, word.c_str(), DICT_WORD_MAX_LEN - 1);
                    hot.word[DICT_WORD_MAX_LEN - 1] = '\0';
                    hot.frequency = freq;
                    hotWords.push_back(hot);
                }
            }
        }
    }
    
    cacheFile.close();
    
    if (strlen(savedLastSearch) > 0) {
        strncpy(lastSearch, savedLastSearch, DICT_INPUT_MAX_LEN - 1);
        lastSearch[DICT_INPUT_MAX_LEN - 1] = '\0';
    }
    
    Serial.println("[DictionaryApp] Cache loaded");
    return true;
}

void DictionaryApp::clearSearchResults() {
    searchResults.clear();
}

void DictionaryApp::clearPages() {
    if (searchPage) {
        lv_obj_del(searchPage);
        searchPage = nullptr;
    }
    if (resultPage) {
        lv_obj_del(resultPage);
        resultPage = nullptr;
    }
    if (detailPage) {
        lv_obj_del(detailPage);
        detailPage = nullptr;
    }
}

void DictionaryApp::onSearchInput() {
    if (!keyboard && searchInput) {
        keyboard = lv_keyboard_create(_screen);
        lv_obj_set_size(keyboard, 320, 120);
        lv_obj_align(keyboard, LV_ALIGN_TOP_MID, 0, 65);
        
        lv_obj_move_foreground(searchInput);
        lv_obj_move_foreground(keyboard);
        
        lv_keyboard_set_textarea(keyboard, searchInput);
        lv_keyboard_set_mode(keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
        
        Serial.println("[DictionaryApp] Keyboard created and configured");
    }
}

void DictionaryApp::keyboard_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    Serial.printf("[DictionaryApp] keyboard_event_cb - code: %d\n", code);
    
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        Serial.println("[DictionaryApp] Keyboard ready/cancel - performing search");
        
        DictionaryApp* app = (DictionaryApp*)lv_event_get_user_data(e);
        if (app && app->keyboard) {
            const char* text = lv_textarea_get_text(app->searchInput);
            Serial.printf("[DictionaryApp] Text from textarea: %s\n", text ? text : "NULL");
            
            lv_obj_del(app->keyboard);
            app->keyboard = nullptr;
            
            if (text && strlen(text) > 0) {
                Serial.println("[DictionaryApp] Calling performSearch");
                app->performSearch();
            } else {
                Serial.println("[DictionaryApp] Text is empty, skipping search");
            }
        }
    }
}

void DictionaryApp::onHistoryButtonClick(const char* word) {
    lv_textarea_set_text(searchInput, word);
    performSearch();
}

void DictionaryApp::onHotWordButtonClick(const char* word) {
    lv_textarea_set_text(searchInput, word);
    performSearch();
}

void DictionaryApp::onResultItemClick(int index) {
    showDetailPage(index);
}

void DictionaryApp::onResultBack() {
    showSearchPage();
}

void DictionaryApp::onDetailBack() {
    showResultPage();
}

void DictionaryApp::addToHistory(const char* word) {
    for (size_t i = 0; i < searchHistory.size(); i++) {
        if (searchHistory[i].equalsIgnoreCase(word)) {
            searchHistory.erase(searchHistory.begin() + i);
            break;
        }
    }
    
    searchHistory.insert(searchHistory.begin(), word);
    
    if (searchHistory.size() > DICT_MAX_HISTORY) {
        searchHistory.pop_back();
    }
}

void DictionaryApp::updateHotWordFrequency(const char* word) {
    bool found = false;
    
    for (size_t i = 0; i < hotWords.size(); i++) {
        if (strcasecmp(hotWords[i].word, word) == 0) {
            hotWords[i].frequency++;
            found = true;
            break;
        }
    }
    
    if (!found) {
        hot_word_t hot;
        strncpy(hot.word, word, DICT_WORD_MAX_LEN - 1);
        hot.word[DICT_WORD_MAX_LEN - 1] = '\0';
        hot.frequency = 1;
        hotWords.push_back(hot);
    }
    
    if (hotWords.size() > 10) {
        std::sort(hotWords.begin(), hotWords.end(), 
            [](const hot_word_t& a, const hot_word_t& b) {
                return a.frequency > b.frequency;
            });
        hotWords.resize(10);
    }
}

void DictionaryApp::search_input_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    
    Serial.printf("[DictionaryApp] search_input_cb - code: %d\n", code);
    
    if (code == LV_EVENT_FOCUSED) {
        Serial.println("[DictionaryApp] Event: FOCUSED - creating keyboard");
        DictionaryApp* app = (DictionaryApp*)lv_event_get_user_data(e);
        if (app) {
            app->onSearchInput();
        }
    } else if (code == LV_EVENT_READY) {
        Serial.println("[DictionaryApp] Event: READY - performing search");
        DictionaryApp* app = (DictionaryApp*)lv_event_get_user_data(e);
        if (app && app->keyboard) {
            Serial.println("[DictionaryApp] Closing keyboard");
            lv_obj_del(app->keyboard);
            app->keyboard = nullptr;
            
            const char* text = lv_textarea_get_text(app->searchInput);
            Serial.printf("[DictionaryApp] Text from textarea: %s\n", text ? text : "NULL");
            
            if (text && strlen(text) > 0) {
                Serial.println("[DictionaryApp] Calling performSearch");
                app->performSearch();
            } else {
                Serial.println("[DictionaryApp] Text is empty, skipping search");
            }
        }
    }
}

void DictionaryApp::search_timer_cb(lv_timer_t* timer) {
    DictionaryApp* app = (DictionaryApp*)timer->user_data;
    if (app) {
        lv_timer_pause(timer);
        if (app->searchPending) {
            app->searchPending = false;
            app->performSearch();
        }
    }
}

void DictionaryApp::history_btn_cb(lv_event_t* e) {
    DictionaryApp* app = (DictionaryApp*)lv_event_get_user_data(e);
    if (app) {
        lv_obj_t* btn = lv_event_get_target(e);
        lv_obj_t* label = lv_obj_get_child(btn, 0);
        const char* word = lv_label_get_text(label);
        app->onHistoryButtonClick(word);
    }
}

void DictionaryApp::hot_word_btn_cb(lv_event_t* e) {
    DictionaryApp* app = (DictionaryApp*)lv_event_get_user_data(e);
    if (app) {
        lv_obj_t* btn = lv_event_get_target(e);
        lv_obj_t* label = lv_obj_get_child(btn, 0);
        const char* word = lv_label_get_text(label);
        app->onHotWordButtonClick(word);
    }
}

void DictionaryApp::result_list_cb(lv_event_t* e) {
    DictionaryApp* app = (DictionaryApp*)lv_event_get_user_data(e);
    if (app) {
        lv_obj_t* list = lv_event_get_target(e);
        lv_obj_t* btn = lv_event_get_target(e);
        
        int index = 0;
        for (int i = 0; i < (int)app->searchResults.size(); i++) {
            lv_obj_t* item = lv_obj_get_child(list, i);
            if (item == btn) {
                index = i;
                break;
            }
        }
        
        app->onResultItemClick(index);
    }
}

void DictionaryApp::result_back_cb(lv_event_t* e) {
    DictionaryApp* app = (DictionaryApp*)lv_event_get_user_data(e);
    if (app) {
        app->onResultBack();
    }
}

void DictionaryApp::detail_back_cb(lv_event_t* e) {
    DictionaryApp* app = (DictionaryApp*)lv_event_get_user_data(e);
    if (app) {
        app->onDetailBack();
    }
}

bool DictionaryApp::onResume() {
    Serial.println("[DictionaryApp] onResume");
    
    if (!BaseApp::onResume()) {
        return false;
    }
    
    loadState();
    
    return true;
}

void DictionaryApp::onPause() {
    Serial.println("[DictionaryApp] onPause");
    
    saveState();
    
    BaseApp::onPause();
}

void DictionaryApp::onUpdate() {
}

app_info_t DictionaryApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "Dictionary", APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, LV_SYMBOL_FILE);
    info.type = APP_TYPE_USER;
    info.enabled = true;
    return info;
}

BaseApp* createDictionaryApp() {
    return new DictionaryApp();
}
