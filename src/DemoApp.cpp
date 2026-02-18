#include "DemoApp.h"
#include "AppManager.h"

DemoApp::DemoApp() : BaseApp("Demo") {
    tabview = nullptr;
    keyboard = nullptr;
    activeTextarea = nullptr;
    _lastUpdateMs = 0;
    _animationPhase = 0;
    _keyboardVisible = false;
    
    barProgress = nullptr;
    arcGauge = nullptr;
    chartWidget = nullptr;
    labelClock = nullptr;
    chartSeries1 = nullptr;
    chartSeries2 = nullptr;
}

bool DemoApp::createUI() {
    lv_obj_t* scr = getScreen();
    
    lv_obj_t* header = lv_obj_create(scr);
    lv_obj_set_size(header, BSP_DISPLAY_WIDTH, 30);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_make(0x20, 0x40, 0x60), 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_style_pad_all(header, 2, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, LV_SYMBOL_PLAY " GUI Demo");
    lv_obj_set_style_text_color(title, lv_color_make(0x00, 0xFF, 0xFF), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_16, 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 5, 0);
    
    lv_obj_t* btnBack = lv_btn_create(header);
    lv_obj_set_size(btnBack, 60, 24);
    lv_obj_align(btnBack, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_add_event_cb(btnBack, back_btn_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btnBack, lv_color_make(0x80, 0x40, 0x40), 0);
    
    lv_obj_t* btnLabel = lv_label_create(btnBack);
    lv_label_set_text(btnLabel, LV_SYMBOL_LEFT " Back");
    lv_obj_set_style_text_font(btnLabel, &lv_font_montserrat_12, 0);
    lv_obj_center(btnLabel);
    
    tabview = lv_tabview_create(scr, LV_DIR_TOP, 25);
    lv_obj_set_size(tabview, BSP_DISPLAY_WIDTH, BSP_DISPLAY_HEIGHT - 30);
    lv_obj_set_pos(tabview, 0, 30);
    lv_obj_set_style_bg_color(tabview, lv_color_black(), 0);
    
    lv_obj_t* tabBtns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tabBtns, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(tabBtns, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(tabBtns, &lv_font_montserrat_10, 0);
    
    lv_obj_t* tab1 = lv_tabview_add_tab(tabview, "Basic");
    lv_obj_set_style_bg_color(tab1, lv_color_make(0x15, 0x15, 0x15), 0);
    createBasicTab(tab1);
    
    lv_obj_t* tab2 = lv_tabview_add_tab(tabview, "Input");
    lv_obj_set_style_bg_color(tab2, lv_color_make(0x15, 0x15, 0x15), 0);
    createInputTab(tab2);
    
    lv_obj_t* tab3 = lv_tabview_add_tab(tabview, "Graph");
    lv_obj_set_style_bg_color(tab3, lv_color_make(0x15, 0x15, 0x15), 0);
    createGraphicsTab(tab3);
    
    lv_obj_t* tab4 = lv_tabview_add_tab(tabview, "List");
    lv_obj_set_style_bg_color(tab4, lv_color_make(0x15, 0x15, 0x15), 0);
    createListTab(tab4);
    
    lv_obj_t* tab5 = lv_tabview_add_tab(tabview, "Color");
    lv_obj_set_style_bg_color(tab5, lv_color_make(0x15, 0x15, 0x15), 0);
    createColorTab(tab5);
    
    keyboard = lv_keyboard_create(scr);
    lv_obj_set_size(keyboard, BSP_DISPLAY_WIDTH, 120);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(keyboard, kb_event_cb, LV_EVENT_KEY, this);
    
    return true;
}

void DemoApp::createBasicTab(lv_obj_t* parent) {
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, BSP_DISPLAY_WIDTH - 10, BSP_DISPLAY_HEIGHT - 100);
    lv_obj_set_style_bg_color(cont, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 5, 0);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(cont, 8, 0);
    
    lv_obj_t* row1 = lv_obj_create(cont);
    lv_obj_set_size(row1, BSP_DISPLAY_WIDTH - 20, 35);
    lv_obj_set_style_bg_opa(row1, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row1, 0, 0);
    lv_obj_set_style_pad_all(row1, 0, 0);
    lv_obj_set_flex_flow(row1, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row1, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(row1, 8, 0);
    
    lv_obj_t* btn1 = lv_btn_create(row1);
    lv_obj_set_size(btn1, 70, 30);
    lv_obj_add_event_cb(btn1, btn_click_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btn1, lv_color_make(0x00, 0x80, 0x00), 0);
    lv_obj_t* lbl1 = lv_label_create(btn1);
    lv_label_set_text(lbl1, LV_SYMBOL_OK " OK");
    lv_obj_center(lbl1);
    
    lv_obj_t* btn2 = lv_btn_create(row1);
    lv_obj_set_size(btn2, 70, 30);
    lv_obj_add_event_cb(btn2, btn_click_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btn2, lv_color_make(0x80, 0x00, 0x00), 0);
    lv_obj_t* lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, LV_SYMBOL_CLOSE " No");
    lv_obj_center(lbl2);
    
    lv_obj_t* btn3 = lv_btn_create(row1);
    lv_obj_set_size(btn3, 70, 30);
    lv_obj_add_event_cb(btn3, btn_click_cb, LV_EVENT_CLICKED, this);
    lv_obj_set_style_bg_color(btn3, lv_color_make(0x00, 0x00, 0x80), 0);
    lv_obj_t* lbl3 = lv_label_create(btn3);
    lv_label_set_text(lbl3, LV_SYMBOL_REFRESH);
    lv_obj_center(lbl3);
    
    lv_obj_t* row2 = lv_obj_create(cont);
    lv_obj_set_size(row2, BSP_DISPLAY_WIDTH - 20, 30);
    lv_obj_set_style_bg_opa(row2, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row2, 0, 0);
    lv_obj_set_style_pad_all(row2, 0, 0);
    lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row2, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(row2, 8, 0);
    
    lv_obj_t* labelSw = lv_label_create(row2);
    lv_label_set_text(labelSw, "Switch:");
    lv_obj_set_style_text_color(labelSw, lv_color_white(), 0);
    
    lv_obj_t* sw1 = lv_switch_create(row2);
    lv_obj_add_event_cb(sw1, switch_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* sw2 = lv_switch_create(row2);
    lv_obj_add_state(sw2, LV_STATE_CHECKED);
    lv_obj_add_event_cb(sw2, switch_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* row3 = lv_obj_create(cont);
    lv_obj_set_size(row3, BSP_DISPLAY_WIDTH - 20, 30);
    lv_obj_set_style_bg_opa(row3, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row3, 0, 0);
    lv_obj_set_style_pad_all(row3, 0, 0);
    lv_obj_set_flex_flow(row3, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row3, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(row3, 8, 0);
    
    lv_obj_t* labelCb = lv_label_create(row3);
    lv_label_set_text(labelCb, "Check:");
    lv_obj_set_style_text_color(labelCb, lv_color_white(), 0);
    
    lv_obj_t* cb1 = lv_checkbox_create(row3);
    lv_checkbox_set_text(cb1, "A");
    
    lv_obj_t* cb2 = lv_checkbox_create(row3);
    lv_checkbox_set_text(cb2, "B");
    lv_obj_add_state(cb2, LV_STATE_CHECKED);
    
    lv_obj_t* cb3 = lv_checkbox_create(row3);
    lv_checkbox_set_text(cb3, "C");
    
    lv_obj_t* row4 = lv_obj_create(cont);
    lv_obj_set_size(row4, BSP_DISPLAY_WIDTH - 20, 35);
    lv_obj_set_style_bg_opa(row4, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row4, 0, 0);
    lv_obj_set_style_pad_all(row4, 0, 0);
    lv_obj_set_flex_flow(row4, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row4, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(row4, 8, 0);
    
    lv_obj_t* labelDd = lv_label_create(row4);
    lv_label_set_text(labelDd, "Select:");
    lv_obj_set_style_text_color(labelDd, lv_color_white(), 0);
    
    lv_obj_t* dd = lv_dropdown_create(row4);
    lv_dropdown_set_options(dd, "Option 1\nOption 2\nOption 3\nOption 4");
    lv_obj_set_width(dd, 120);
    lv_obj_add_event_cb(dd, dropdown_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* row5 = lv_obj_create(cont);
    lv_obj_set_size(row5, BSP_DISPLAY_WIDTH - 20, 35);
    lv_obj_set_style_bg_opa(row5, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row5, 0, 0);
    lv_obj_set_style_pad_all(row5, 0, 0);
    lv_obj_set_flex_flow(row5, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row5, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_column(row5, 8, 0);
    
    lv_obj_t* labelSlider = lv_label_create(row5);
    lv_label_set_text(labelSlider, "Slider:");
    lv_obj_set_style_text_color(labelSlider, lv_color_white(), 0);
    
    lv_obj_t* slider = lv_slider_create(row5);
    lv_obj_set_width(slider, 120);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
    
    lv_obj_t* sliderVal = lv_label_create(row5);
    lv_label_set_text(sliderVal, "50");
    lv_obj_set_style_text_color(sliderVal, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_add_event_cb(slider, slider_cb, LV_EVENT_VALUE_CHANGED, sliderVal);
    
    lv_obj_t* row6 = lv_obj_create(cont);
    lv_obj_set_size(row6, BSP_DISPLAY_WIDTH - 20, 25);
    lv_obj_set_style_bg_opa(row6, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row6, 0, 0);
    lv_obj_set_style_pad_all(row6, 0, 0);
    
    lv_obj_t* labelSymbols = lv_label_create(row6);
    lv_label_set_text(labelSymbols, 
        LV_SYMBOL_WIFI " " LV_SYMBOL_BLUETOOTH " " LV_SYMBOL_GPS " " 
        LV_SYMBOL_BATTERY_FULL " " LV_SYMBOL_SD_CARD " " LV_SYMBOL_USB);
    lv_obj_set_style_text_color(labelSymbols, lv_color_make(0x00, 0xC0, 0xC0), 0);
    lv_obj_set_style_text_font(labelSymbols, &lv_font_montserrat_16, 0);
    lv_obj_center(labelSymbols);
}

void DemoApp::createInputTab(lv_obj_t* parent) {
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, BSP_DISPLAY_WIDTH - 10, BSP_DISPLAY_HEIGHT - 100);
    lv_obj_set_style_bg_color(cont, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 8, 0);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(cont, 10, 0);
    
    lv_obj_t* labelHint = lv_label_create(cont);
    lv_label_set_text(labelHint, "Tap text area to show keyboard:");
    lv_obj_set_style_text_color(labelHint, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    
    lv_obj_t* ta1 = lv_textarea_create(cont);
    lv_obj_set_width(ta1, BSP_DISPLAY_WIDTH - 30);
    lv_textarea_set_one_line(ta1, true);
    lv_textarea_set_placeholder_text(ta1, "Enter your name...");
    lv_textarea_set_max_length(ta1, 32);
    lv_obj_add_event_cb(ta1, textarea_cb, LV_EVENT_FOCUSED, this);
    lv_obj_add_event_cb(ta1, textarea_cb, LV_EVENT_DEFOCUSED, this);
    lv_obj_set_style_bg_color(ta1, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(ta1, lv_color_white(), 0);
    
    lv_obj_t* ta2 = lv_textarea_create(cont);
    lv_obj_set_width(ta2, BSP_DISPLAY_WIDTH - 30);
    lv_obj_set_height(ta2, 60);
    lv_textarea_set_placeholder_text(ta2, "Enter description...");
    lv_textarea_set_max_length(ta2, 128);
    lv_obj_add_event_cb(ta2, textarea_cb, LV_EVENT_FOCUSED, this);
    lv_obj_add_event_cb(ta2, textarea_cb, LV_EVENT_DEFOCUSED, this);
    lv_obj_set_style_bg_color(ta2, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(ta2, lv_color_white(), 0);
    
    lv_obj_t* btnm = lv_btnmatrix_create(cont);
    lv_obj_set_size(btnm, BSP_DISPLAY_WIDTH - 30, 80);
    static const char* btnmMap[] = {
        "1", "2", "3", "\n",
        "4", "5", "6", "\n",
        "7", "8", "9", "\n",
        "*", "0", "#", ""
    };
    lv_btnmatrix_set_map(btnm, btnmMap);
    lv_obj_set_style_bg_color(btnm, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btnm, lv_color_white(), 0);
    
    lv_obj_t* labelInfo = lv_label_create(cont);
    lv_label_set_text(labelInfo, "Keyboard supports:\n- Text input\n- Numbers\n- Special chars");
    lv_obj_set_style_text_color(labelInfo, lv_color_make(0x80, 0x80, 0x80), 0);
    lv_obj_set_style_text_font(labelInfo, &lv_font_montserrat_10, 0);
}

void DemoApp::createGraphicsTab(lv_obj_t* parent) {
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, BSP_DISPLAY_WIDTH - 10, BSP_DISPLAY_HEIGHT - 100);
    lv_obj_set_style_bg_color(cont, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 8, 0);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(cont, 8, 0);
    lv_obj_set_style_pad_column(cont, 8, 0);
    
    lv_obj_t* panel1 = lv_obj_create(cont);
    lv_obj_set_size(panel1, 145, 90);
    lv_obj_set_style_bg_color(panel1, lv_color_make(0x25, 0x25, 0x25), 0);
    lv_obj_set_style_border_width(panel1, 1, 0);
    lv_obj_set_style_border_color(panel1, lv_color_make(0x40, 0x40, 0x40), 0);
    lv_obj_set_style_pad_all(panel1, 5, 0);
    
    labelClock = lv_label_create(panel1);
    lv_label_set_text(labelClock, "00:00:00");
    lv_obj_set_style_text_color(labelClock, lv_color_make(0x00, 0xFF, 0x00), 0);
    lv_obj_set_style_text_font(labelClock, &lv_font_montserrat_20, 0);
    lv_obj_align(labelClock, LV_ALIGN_TOP_MID, 0, 0);
    
    barProgress = lv_bar_create(panel1);
    lv_obj_set_size(barProgress, 130, 10);
    lv_bar_set_value(barProgress, 0, LV_ANIM_OFF);
    lv_obj_align(barProgress, LV_ALIGN_BOTTOM_MID, 0, 0);
    
    lv_obj_t* panel2 = lv_obj_create(cont);
    lv_obj_set_size(panel2, 145, 90);
    lv_obj_set_style_bg_color(panel2, lv_color_make(0x25, 0x25, 0x25), 0);
    lv_obj_set_style_border_width(panel2, 1, 0);
    lv_obj_set_style_border_color(panel2, lv_color_make(0x40, 0x40, 0x40), 0);
    lv_obj_set_style_pad_all(panel2, 5, 0);
    
    lv_obj_t* labelArc = lv_label_create(panel2);
    lv_label_set_text(labelArc, "Gauge");
    lv_obj_set_style_text_color(labelArc, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelArc, &lv_font_montserrat_10, 0);
    lv_obj_align(labelArc, LV_ALIGN_TOP_MID, 0, 0);
    
    arcGauge = lv_arc_create(panel2);
    lv_obj_set_size(arcGauge, 60, 60);
    lv_arc_set_range(arcGauge, 0, 100);
    lv_arc_set_value(arcGauge, 0);
    lv_arc_set_bg_angles(arcGauge, 0, 270);
    lv_obj_set_style_arc_color(arcGauge, lv_color_make(0x00, 0xC0, 0xC0), LV_PART_INDICATOR);
    lv_obj_align(arcGauge, LV_ALIGN_CENTER, 0, 8);
    
    lv_obj_t* panel3 = lv_obj_create(cont);
    lv_obj_set_size(panel3, 145, 90);
    lv_obj_set_style_bg_color(panel3, lv_color_make(0x25, 0x25, 0x25), 0);
    lv_obj_set_style_border_width(panel3, 1, 0);
    lv_obj_set_style_border_color(panel3, lv_color_make(0x40, 0x40, 0x40), 0);
    lv_obj_set_style_pad_all(panel3, 5, 0);
    
    lv_obj_t* labelMeter = lv_label_create(panel3);
    lv_label_set_text(labelMeter, "Meter");
    lv_obj_set_style_text_color(labelMeter, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelMeter, &lv_font_montserrat_10, 0);
    lv_obj_align(labelMeter, LV_ALIGN_TOP_MID, 0, 0);
    
    lv_obj_t* meter = lv_meter_create(panel3);
    lv_obj_set_size(meter, 65, 65);
    lv_obj_align(meter, LV_ALIGN_CENTER, 0, 8);
    lv_obj_remove_style(meter, NULL, LV_PART_MAIN);
    
    lv_meter_scale_t* scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 5, 2, 10, lv_color_make(0x80, 0x80, 0x80));
    lv_meter_set_scale_major_ticks(meter, scale, 1, 4, 15, lv_color_white(), 10);
    
    lv_meter_indicator_t* indic = lv_meter_add_needle_line(meter, scale, 3, lv_color_make(0xFF, 0x00, 0x00), -5);
    lv_meter_set_indicator_value(meter, indic, 60);
    
    lv_obj_t* panel4 = lv_obj_create(cont);
    lv_obj_set_size(panel4, 145, 90);
    lv_obj_set_style_bg_color(panel4, lv_color_make(0x25, 0x25, 0x25), 0);
    lv_obj_set_style_border_width(panel4, 1, 0);
    lv_obj_set_style_border_color(panel4, lv_color_make(0x40, 0x40, 0x40), 0);
    lv_obj_set_style_pad_all(panel4, 5, 0);
    
    lv_obj_t* labelSpin = lv_label_create(panel4);
    lv_label_set_text(labelSpin, "Spinner");
    lv_obj_set_style_text_color(labelSpin, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelSpin, &lv_font_montserrat_10, 0);
    lv_obj_align(labelSpin, LV_ALIGN_TOP_MID, 0, 0);
    
    lv_obj_t* spinner = lv_spinner_create(panel4, 1000, 60);
    lv_obj_set_size(spinner, 50, 50);
    lv_obj_set_style_arc_width(spinner, 4, 0);
    lv_obj_set_style_arc_width(spinner, 4, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(spinner, lv_color_make(0xFF, 0x80, 0x00), LV_PART_INDICATOR);
    lv_obj_align(spinner, LV_ALIGN_CENTER, 0, 8);
    
    lv_obj_t* panel5 = lv_obj_create(cont);
    lv_obj_set_size(panel5, BSP_DISPLAY_WIDTH - 30, 70);
    lv_obj_set_style_bg_color(panel5, lv_color_make(0x25, 0x25, 0x25), 0);
    lv_obj_set_style_border_width(panel5, 1, 0);
    lv_obj_set_style_border_color(panel5, lv_color_make(0x40, 0x40, 0x40), 0);
    lv_obj_set_style_pad_all(panel5, 5, 0);
    
    lv_obj_t* labelChart = lv_label_create(panel5);
    lv_label_set_text(labelChart, "Chart");
    lv_obj_set_style_text_color(labelChart, lv_color_make(0xAA, 0xAA, 0xAA), 0);
    lv_obj_set_style_text_font(labelChart, &lv_font_montserrat_10, 0);
    lv_obj_align(labelChart, LV_ALIGN_TOP_LEFT, 0, 0);
    
    chartWidget = lv_chart_create(panel5);
    lv_obj_set_size(chartWidget, BSP_DISPLAY_WIDTH - 50, 45);
    lv_obj_align(chartWidget, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_chart_set_type(chartWidget, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chartWidget, 10);
    lv_chart_set_range(chartWidget, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_bg_color(chartWidget, lv_color_make(0x20, 0x20, 0x20), 0);
    
    chartSeries1 = lv_chart_add_series(chartWidget, lv_color_make(0x00, 0xFF, 0x00), LV_CHART_AXIS_PRIMARY_Y);
    chartSeries2 = lv_chart_add_series(chartWidget, lv_color_make(0xFF, 0x00, 0xFF), LV_CHART_AXIS_PRIMARY_Y);
    
    for (int i = 0; i < 10; i++) {
        lv_chart_set_next_value(chartWidget, chartSeries1, 30 + rand() % 40);
        lv_chart_set_next_value(chartWidget, chartSeries2, 50 + rand() % 30);
    }
}

void DemoApp::createListTab(lv_obj_t* parent) {
    lv_obj_t* list = lv_list_create(parent);
    lv_obj_set_size(list, BSP_DISPLAY_WIDTH - 10, BSP_DISPLAY_HEIGHT - 100);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_bg_color(list, lv_color_make(0x20, 0x20, 0x20), 0);
    
    lv_obj_t* btn;
    
    btn = lv_list_add_btn(list, LV_SYMBOL_FILE, "File Manager");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_DIRECTORY, "Documents");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_IMAGE, "Pictures");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_VIDEO, "Videos");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_AUDIO, "Music");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_DOWNLOAD, "Downloads");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_SD_CARD, "SD Card");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_USB, "USB Storage");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_WIFI, "Network");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_BLUETOOTH, "Bluetooth");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_SETTINGS, "Settings");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
    
    btn = lv_list_add_btn(list, LV_SYMBOL_POWER, "Power");
    lv_obj_set_style_bg_color(btn, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_text_color(btn, lv_color_white(), 0);
    lv_obj_add_event_cb(btn, list_btn_cb, LV_EVENT_CLICKED, this);
}

void DemoApp::createColorTab(lv_obj_t* parent) {
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, BSP_DISPLAY_WIDTH - 10, BSP_DISPLAY_HEIGHT - 100);
    lv_obj_set_style_bg_color(cont, lv_color_make(0x20, 0x20, 0x20), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_pad_all(cont, 8, 0);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(cont, 10, 0);
    
    lv_obj_t* labelCw = lv_label_create(cont);
    lv_label_set_text(labelCw, "Color Wheel:");
    lv_obj_set_style_text_color(labelCw, lv_color_white(), 0);
    
    lv_obj_t* cw = lv_colorwheel_create(cont, true);
    lv_obj_set_size(cw, 100, 100);
    lv_obj_add_event_cb(cw, colorwheel_cb, LV_EVENT_VALUE_CHANGED, this);
    
    lv_obj_t* colorPreview = lv_obj_create(cont);
    lv_obj_set_size(colorPreview, BSP_DISPLAY_WIDTH - 30, 30);
    lv_obj_set_style_bg_color(colorPreview, lv_color_make(0xFF, 0x00, 0x00), 0);
    lv_obj_set_style_border_width(colorPreview, 1, 0);
    lv_obj_set_style_border_color(colorPreview, lv_color_white(), 0);
    lv_obj_add_flag(colorPreview, LV_OBJ_FLAG_USER_1);
    
    lv_obj_t* labelPalette = lv_label_create(cont);
    lv_label_set_text(labelPalette, "Color Palette:");
    lv_obj_set_style_text_color(labelPalette, lv_color_white(), 0);
    
    lv_obj_t* paletteCont = lv_obj_create(cont);
    lv_obj_set_size(paletteCont, BSP_DISPLAY_WIDTH - 30, 50);
    lv_obj_set_style_bg_opa(paletteCont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(paletteCont, 0, 0);
    lv_obj_set_style_pad_all(paletteCont, 2, 0);
    lv_obj_set_flex_flow(paletteCont, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(paletteCont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(paletteCont, 4, 0);
    lv_obj_set_style_pad_column(paletteCont, 4, 0);
    
    lv_color_t colors[] = {
        lv_color_make(0xFF, 0x00, 0x00),
        lv_color_make(0xFF, 0x80, 0x00),
        lv_color_make(0xFF, 0xFF, 0x00),
        lv_color_make(0x80, 0xFF, 0x00),
        lv_color_make(0x00, 0xFF, 0x00),
        lv_color_make(0x00, 0xFF, 0x80),
        lv_color_make(0x00, 0xFF, 0xFF),
        lv_color_make(0x00, 0x80, 0xFF),
        lv_color_make(0x00, 0x00, 0xFF),
        lv_color_make(0x80, 0x00, 0xFF),
        lv_color_make(0xFF, 0x00, 0xFF),
        lv_color_make(0xFF, 0x00, 0x80),
    };
    
    for (int i = 0; i < 12; i++) {
        lv_obj_t* colorBtn = lv_btn_create(paletteCont);
        lv_obj_set_size(colorBtn, 22, 22);
        lv_obj_set_style_bg_color(colorBtn, colors[i], 0);
        lv_obj_set_style_radius(colorBtn, 4, 0);
        lv_obj_add_event_cb(colorBtn, colorwheel_cb, LV_EVENT_CLICKED, this);
    }
}

void DemoApp::showKeyboard(lv_obj_t* textarea) {
    if (keyboard && textarea) {
        activeTextarea = textarea;
        lv_keyboard_set_textarea(keyboard, textarea);
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        _keyboardVisible = true;
    }
}

void DemoApp::hideKeyboard() {
    if (keyboard) {
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(keyboard, NULL);
        activeTextarea = nullptr;
        _keyboardVisible = false;
    }
}

void DemoApp::btn_click_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_color_t color = lv_obj_get_style_bg_color(btn, 0);
    
    uint16_t c16 = lv_color_to16(color);
    uint8_t r = (c16 >> 11) & 0x1F;
    uint8_t g = (c16 >> 5) & 0x3F;
    uint8_t b = c16 & 0x1F;
    bsp_rgb_led_set(r << 3, g << 2, b << 3);
    
    Serial.println("[Demo] Button clicked");
}

void DemoApp::slider_cb(lv_event_t* e) {
    lv_obj_t* slider = lv_event_get_target(e);
    lv_obj_t* label = (lv_obj_t*)lv_event_get_user_data(e);
    
    if (label) {
        int value = lv_slider_get_value(slider);
        lv_label_set_text_fmt(label, "%d", value);
    }
}

void DemoApp::switch_cb(lv_event_t* e) {
    lv_obj_t* sw = lv_event_get_target(e);
    bool checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    Serial.printf("[Demo] Switch: %s\n", checked ? "ON" : "OFF");
}

void DemoApp::dropdown_cb(lv_event_t* e) {
    lv_obj_t* dd = lv_event_get_target(e);
    int selected = lv_dropdown_get_selected(dd);
    Serial.printf("[Demo] Dropdown: option %d\n", selected);
}

void DemoApp::textarea_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    DemoApp* app = (DemoApp*)lv_event_get_user_data(e);
    lv_obj_t* ta = lv_event_get_target(e);
    
    if (code == LV_EVENT_FOCUSED) {
        app->showKeyboard(ta);
    } else if (code == LV_EVENT_DEFOCUSED) {
        if (app->activeTextarea == ta) {
            app->hideKeyboard();
        }
    }
}

void DemoApp::kb_event_cb(lv_event_t* e) {
    lv_event_code_t code = lv_event_get_code(e);
    DemoApp* app = (DemoApp*)lv_event_get_user_data(e);
    
    if (code == LV_EVENT_CANCEL) {
        app->hideKeyboard();
    }
}

void DemoApp::colorwheel_cb(lv_event_t* e) {
    lv_obj_t* cw = lv_event_get_target(e);
    DemoApp* app = (DemoApp*)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    
    lv_color_t color = lv_colorwheel_get_rgb(cw);
    
    lv_obj_t* parent = lv_obj_get_parent(cw);
    lv_obj_t* preview = lv_obj_get_child(parent, 2);
    
    if (preview && lv_obj_has_flag(preview, LV_OBJ_FLAG_USER_1)) {
        lv_obj_set_style_bg_color(preview, color, 0);
    }
    
    uint16_t c16 = lv_color_to16(color);
    uint8_t r = (c16 >> 11) & 0x1F;
    uint8_t g = (c16 >> 5) & 0x3F;
    uint8_t b = c16 & 0x1F;
    bsp_rgb_led_set(r << 3, g << 2, b << 3);
}

void DemoApp::list_btn_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    lv_obj_t* label = lv_obj_get_child(btn, 1);
    const char* text = lv_label_get_text(label);
    Serial.printf("[Demo] List item: %s\n", text);
}

void DemoApp::back_btn_cb(lv_event_t* e) {
    DemoApp* app = (DemoApp*)lv_event_get_user_data(e);
    app->hideKeyboard();
    AppMgr.switchToHome();
}

void DemoApp::onUpdate() {
    uint32_t now = millis();
    
    if (now - _lastUpdateMs < 200) {
        return;
    }
    _lastUpdateMs = now;
    
    _animationPhase = (_animationPhase + 1) % 100;
    
    if (labelClock) {
        uint32_t secs = now / 1000;
        uint32_t hours = (secs / 3600) % 24;
        uint32_t mins = (secs / 60) % 60;
        uint32_t s = secs % 60;
        lv_label_set_text_fmt(labelClock, "%02u:%02u:%02u", hours, mins, s);
    }
    
    if (barProgress) {
        int progress = (_animationPhase * 100) / 100;
        lv_bar_set_value(barProgress, progress, LV_ANIM_OFF);
    }
    
    if (arcGauge) {
        int gaugeValue = 50 + (int)(sin(_animationPhase * 0.1) * 40);
        lv_arc_set_value(arcGauge, gaugeValue);
    }
    
    if (chartWidget && _animationPhase % 5 == 0) {
        lv_chart_set_next_value(chartWidget, chartSeries1, 30 + rand() % 40);
        lv_chart_set_next_value(chartWidget, chartSeries2, 50 + rand() % 30);
    }
}

app_info_t DemoApp::getInfo() const {
    app_info_t info;
    strncpy(info.name, "Demo", APP_NAME_MAX_LEN - 1);
    info.name[APP_NAME_MAX_LEN - 1] = '\0';
    strcpy(info.icon, LV_SYMBOL_PLAY);
    info.type = APP_TYPE_USER;
    info.enabled = true;
    return info;
}

BaseApp* createDemoApp() {
    return new DemoApp();
}
