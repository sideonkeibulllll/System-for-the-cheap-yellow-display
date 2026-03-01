#include "WordCardApp.h"
#include "AppManager.h"
#include "Storage.h"
#include "LvZhFont.h"
#include <SD.h>
#include <SPIFFS.h>

WordCardApp::WordCardApp() : BaseApp("WordCard") {
    labelWord = nullptr;
    labelPhonetic = nullptr;
    labelMeaning = nullptr;
    btnNext = nullptr;
    labelProgress = nullptr;
    
    _wordCount = 0;
    _currentIndex = 0;
    _showingFront = true;
    
    memset(_words, 0, sizeof(_words));
}

bool WordCardApp::loadWordsFromJson() {
    File file;
    bool fromSD = false;
    
    if (SD.begin()) {
        file = SD.open(WORD_FILE_PATH);
        if (file) {
            fromSD = true;
            Serial.println("[WordCard] Loading from SD card");
        }
    }
    
    if (!file && SPIFFS.begin()) {
        file = SPIFFS.open(WORD_FILE_PATH);
        if (file) {
            Serial.println("[WordCard] Loading from SPIFFS");
        }
    }
    
    if (!file) {
        Serial.println("[WordCard] word.json not found, using default words");
        strcpy(_words[0].word, "accommodation");
        strcpy(_words[0].phonetic, "/əˌkɒməˈdeɪʃn/");
        strcpy(_words[0].meaning, "住宿；膳宿");
        
        strcpy(_words[1].word, "unfortunately");
        strcpy(_words[1].phonetic, "/ʌnˈfɔːtʃənətli/");
        strcpy(_words[1].meaning, "不幸地；遗憾地");
        
        strcpy(_words[2].word, "communication");
        strcpy(_words[2].phonetic, "/kəˌmjuːnɪˈkeɪʃn/");
        strcpy(_words[2].meaning, "沟通；交流；通信");
        
        _wordCount = 3;
        return true;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    
    if (error) {
        Serial.printf("[WordCard] JSON parse error: %s\n", error.c_str());
        return false;
    }
    
    JsonArray arr = doc.as<JsonArray>();
    _wordCount = 0;
    
    for (JsonObject obj : arr) {
        if (_wordCount >= MAX_WORDS) break;
        
        const char* word = obj["word"];
        const char* meaning = obj["meaning"];
        const char* phonetic = obj["phonetic"];
        
        if (word && meaning) {
            strncpy(_words[_wordCount].word, word, WORD_MAX_LEN - 1);
            strncpy(_words[_wordCount].meaning, meaning, MEANING_MAX_LEN - 1);
            if (phonetic) {
                strncpy(_words[_wordCount].phonetic, phonetic, PHONETIC_MAX_LEN - 1);
            }
            _wordCount++;
        }
    }
    
    Serial.printf("[WordCard] Loaded %d words\n", _wordCount);
    return _wordCount > 0;
}

bool WordCardApp::createUI() {
    loadWordsFromJson();
    
    lv_obj_t* scr = getScreen();
    
    lv_obj_t* titleContainer = lv_obj_create(scr);
    lv_obj_set_size(titleContainer, BSP_DISPLAY_WIDTH, 30);
    lv_obj_set_style_bg_color(titleContainer, lv_color_make(0x20, 0x20, 0x30), 0);
    lv_obj_set_style_border_width(titleContainer, 0, 0);
    lv_obj_set_style_pad_all(titleContainer, 0, 0);
    lv_obj_align(titleContainer, LV_ALIGN_TOP_MID, 0, 0);
    
    lv_obj_t* title = lv_label_create(titleContainer);
    lv_label_set_text(title, LV_SYMBOL_EDIT " Word Card");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 10, 0);
    
    labelProgress = lv_label_create(titleContainer);
    lv_label_set_text(labelProgress, "1/10");
    lv_obj_set_style_text_color(labelProgress, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelProgress, &lv_font_montserrat_14, 0);
    lv_obj_align(labelProgress, LV_ALIGN_RIGHT_MID, -10, 0);
    
    lv_obj_t* cardContainer = lv_obj_create(scr);
    lv_obj_set_size(cardContainer, BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT - 80);
    lv_obj_set_style_bg_color(cardContainer, lv_color_make(0x15, 0x15, 0x25), 0);
    lv_obj_set_style_border_width(cardContainer, 0, 0);
    lv_obj_set_style_radius(cardContainer, 0, 0);
    lv_obj_set_style_pad_all(cardContainer, 0, 0);
    lv_obj_align(cardContainer, LV_ALIGN_TOP_MID, 0, 30);
    
    labelWord = lv_label_create(cardContainer);
    lv_label_set_text(labelWord, "word");
    lv_obj_set_style_text_color(labelWord, lv_color_make(0xFF, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelWord, &lv_font_montserrat_28, 0);
    lv_obj_align(labelWord, LV_ALIGN_TOP_MID, 0, 30);
    
    labelPhonetic = lv_label_create(cardContainer);
    lv_label_set_text(labelPhonetic, "/phonetic/");
    lv_obj_set_style_text_color(labelPhonetic, lv_color_make(0x80, 0xC0, 0xFF), 0);
    lv_obj_set_style_text_font(labelPhonetic, &lv_font_montserrat_16, 0);
    lv_obj_align(labelPhonetic, LV_ALIGN_TOP_MID, 0, 75);
    
    labelMeaning = lv_label_create(cardContainer);
    lv_label_set_text(labelMeaning, "meaning");
    lv_obj_set_style_text_color(labelMeaning, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    if (LvZhFontMgr.isInitialized()) {
        lv_obj_set_style_text_font(labelMeaning, LvZhFontMgr.getFont(), 0);
    } else {
        lv_obj_set_style_text_font(labelMeaning, &lv_font_montserrat_20, 0);
    }
    lv_obj_align(labelMeaning, LV_ALIGN_TOP_MID, 0, 110);
    
    btnNext = lv_btn_create(scr);
    lv_obj_set_size(btnNext, BSP_DISPLAY_WIDTH, 50);
    lv_obj_align(btnNext, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_event_cb(btnNext, btn_next_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnNext, lv_color_make(0x00, 0x60, 0x40), 0);
    lv_obj_set_style_radius(btnNext, 0, 0);
    
    lv_obj_t* nextLabel = lv_label_create(btnNext);
    lv_label_set_text(nextLabel, LV_SYMBOL_RIGHT " Next");
    lv_obj_set_style_text_font(nextLabel, &lv_font_montserrat_20, 0);
    lv_obj_center(nextLabel);
    
    updateDisplay();
    
    return true;
}

void WordCardApp::updateDisplay() {
    if (_wordCount == 0) {
        lv_label_set_text(labelWord, "No words");
        lv_label_set_text(labelPhonetic, "");
        lv_label_set_text(labelMeaning, "");
        return;
    }
    
    word_card_t* card = &_words[_currentIndex];
    
    lv_label_set_text(labelWord, card->word);
    lv_label_set_text(labelPhonetic, card->phonetic);
    
    if (_showingFront) {
        lv_label_set_text(labelMeaning, "");
    } else {
        lv_label_set_text(labelMeaning, card->meaning);
    }
    
    lv_label_set_text_fmt(labelProgress, "%d / %d", _currentIndex + 1, _wordCount);
}

void WordCardApp::showFront() {
    _showingFront = true;
    updateDisplay();
}

void WordCardApp::showBack() {
    _showingFront = false;
    updateDisplay();
}

void WordCardApp::btn_next_cb(lv_event_t* e) {
    WordCardApp* app = (WordCardApp*)lv_event_get_user_data(e);
    
    if (app->_showingFront) {
        app->showBack();
    } else {
        app->_currentIndex++;
        if (app->_currentIndex >= app->_wordCount) {
            app->_currentIndex = 0;
        }
        app->showFront();
    }
    
    Power.resetIdleTimer();
}

void WordCardApp::onUpdate() {
}

void WordCardApp::saveState() {
    Serial.printf("[WordCard] Saving state: index=%d, front=%d\n", _currentIndex, _showingFront);
}

bool WordCardApp::loadState() {
    Serial.printf("[WordCard] Loaded state: index=%d, front=%d\n", _currentIndex, _showingFront);
    updateDisplay();
    return true;
}

app_info_t WordCardApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "WordCard", APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, LV_SYMBOL_EDIT);
    info.type = APP_TYPE_USER;
    info.enabled = true;
    return info;
}

BaseApp* createWordCardApp() {
    return new WordCardApp();
}
