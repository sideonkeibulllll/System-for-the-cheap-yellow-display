#ifndef WORDCARDAPP_H
#define WORDCARDAPP_H

#include "AppManager.h"
#include "BSP.h"
#include "PowerManager.h"
#include <ArduinoJson.h>

#define WORD_FILE_PATH "/word.json"
#define MAX_WORDS 50
#define WORD_MAX_LEN 64
#define MEANING_MAX_LEN 128
#define PHONETIC_MAX_LEN 64

typedef struct {
    char word[WORD_MAX_LEN];
    char meaning[MEANING_MAX_LEN];
    char phonetic[PHONETIC_MAX_LEN];
} word_card_t;

class WordCardApp : public BaseApp {
private:
    lv_obj_t* labelWord;
    lv_obj_t* labelPhonetic;
    lv_obj_t* labelMeaning;
    lv_obj_t* btnNext;
    lv_obj_t* labelProgress;
    
    word_card_t _words[MAX_WORDS];
    int _wordCount;
    int _currentIndex;
    bool _showingFront;
    
    static void btn_next_cb(lv_event_t* e);
    
    bool loadWordsFromJson();
    void updateDisplay();
    void showFront();
    void showBack();
    
protected:
    virtual bool createUI() override;
    virtual void saveState() override;
    virtual bool loadState() override;
    
public:
    WordCardApp();
    virtual ~WordCardApp() {}
    
    virtual void onUpdate() override;
    virtual app_info_t getInfo() const override;
};

BaseApp* createWordCardApp();

#endif
