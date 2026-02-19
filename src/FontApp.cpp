#include "FontApp.h"
#include "AppManager.h"

FontApp::FontApp() : BaseApp("Font") {
    fontList = nullptr;
    btnBack = nullptr;
}

bool FontApp::createUI() {
    lv_obj_t* scr = getScreen();
    
    lv_obj_t* header = lv_obj_create(scr);
    lv_obj_set_size(header, BSP_DISPLAY_WIDTH, 30);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(0x30, 0x50, 0x70), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 2, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, LV_SYMBOL_EDIT " Font Viewer");
    lv_obj_set_style_text_color(title, lv_color_make(0xFF, 0xFF, 0xFF), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 5, 0);
    
    btnBack = lv_btn_create(header);
    lv_obj_set_size(btnBack, 60, 24);
    lv_obj_align(btnBack, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_add_event_cb(btnBack, back_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnBack, lv_color_make(0x80, 0x40, 0x40), 0);
    
    lv_obj_t* btnLabel = lv_label_create(btnBack);
    lv_label_set_text(btnLabel, LV_SYMBOL_LEFT " Back");
    lv_obj_center(btnLabel);
    
    fontList = lv_list_create(scr);
    lv_obj_set_size(fontList, BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT - 35);
    lv_obj_set_pos(fontList, 0, 32);
    lv_obj_set_style_bg_color(fontList, lv_color_make(0x18, 0x18, 0x18), 0);
    
    typedef struct {
        const char* name;
        const lv_font_t* font;
    } font_entry_t;
    
    font_entry_t fonts[] = {
        { "8pt - Tiny", &lv_font_montserrat_8 },
        { "10pt - Small", &lv_font_montserrat_10 },
        { "12pt - Normal", &lv_font_montserrat_12 },
        { "14pt - Medium", &lv_font_montserrat_14 },
        { "16pt - Large", &lv_font_montserrat_16 },
        { "18pt - Title", &lv_font_montserrat_18 },
        { "20pt - Header", &lv_font_montserrat_20 },
        { "22pt - Big", &lv_font_montserrat_22 },
        { "24pt - Large", &lv_font_montserrat_24 },
        { "26pt - XLarge", &lv_font_montserrat_26 },
        { "28pt - Huge", &lv_font_montserrat_28 },
        { "30pt - Giant", &lv_font_montserrat_30 },
    };
    
    int fontCount = sizeof(fonts) / sizeof(fonts[0]);
    
    for (int i = 0; i < fontCount; i++) {
        lv_obj_t* item = lv_list_add_btn(fontList, NULL, NULL);
        lv_obj_set_style_bg_color(item, lv_color_make(0x25, 0x25, 0x25), 0);
        lv_obj_set_style_bg_color(item, lv_color_make(0x35, 0x35, 0x35), LV_STATE_PRESSED);
        lv_obj_set_style_border_width(item, 0, 0);
        lv_obj_set_style_pad_all(item, 8, 0);
        lv_obj_set_height(item, LV_SIZE_CONTENT);
        
        lv_obj_t* sizeLabel = lv_label_create(item);
        lv_label_set_text(sizeLabel, fonts[i].name);
        lv_obj_set_style_text_color(sizeLabel, lv_color_make(0x88, 0x88, 0x88), 0);
        lv_obj_set_style_text_font(sizeLabel, &lv_font_montserrat_10, 0);
        lv_obj_align(sizeLabel, LV_ALIGN_TOP_LEFT, 0, 0);
        
        lv_obj_t* sampleLabel = lv_label_create(item);
        lv_label_set_text(sampleLabel, "AaBbCc 123");
        lv_obj_set_style_text_color(sampleLabel, lv_color_make(0xFF, 0xFF, 0xFF), 0);
        lv_obj_set_style_text_font(sampleLabel, fonts[i].font, 0);
        lv_obj_align(sampleLabel, LV_ALIGN_TOP_LEFT, 0, 14);
    }
    
    return true;
}

void FontApp::back_btn_cb(lv_event_t* e) {
    AppMgr.switchToHome();
}

app_info_t FontApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "Font", APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, LV_SYMBOL_EDIT);
    info.type = APP_TYPE_USER;
    info.enabled = true;
    return info;
}

BaseApp* createFontApp() {
    return new FontApp();
}
