#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

const char* fontNames[] = {
  "GLCD",
  "Font2", 
  "Font4",
  "Font6",
  "Font7",
  "Font8"
};

const uint8_t fontIDs[] = {
  1,
  2,
  4,
  6,
  7,
  8
};

const char* previewText = "ABCDEFGabcdefg0123456789";
const char* sampleTextCN = "Font Preview";

int currentFont = 0;
unsigned long lastChange = 0;
const unsigned long changeInterval = 3000;

void showFontPreview(int fontIndex);

void setup() {
  Serial.begin(115200);
  Serial.println("Font Preview Starting...");
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  
  pinMode(21, OUTPUT);
  digitalWrite(21, HIGH);
  
  showFontPreview(currentFont);
  lastChange = millis();
}

void loop() {
  if (millis() - lastChange >= changeInterval) {
    currentFont = (currentFont + 1) % 6;
    showFontPreview(currentFont);
    lastChange = millis();
  }
}

void showFontPreview(int fontIndex) {
  tft.fillScreen(TFT_BLACK);
  
  int barHeight = 30;
  tft.fillRect(0, 0, SCREEN_WIDTH, barHeight, TFT_NAVY);
  
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(1);
  tft.drawString("Font Preview", SCREEN_WIDTH / 2, 10, 2);
  
  tft.fillRect(0, barHeight, SCREEN_WIDTH, 2, TFT_CYAN);
  
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setTextSize(1);
  
  int yPos = barHeight + 15;
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawString("Font: ", 10, yPos, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(fontNames[fontIndex], 60, yPos, 2);
  
  yPos += 30;
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Preview:", 10, yPos, 2);
  
  yPos += 25;
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  
  uint8_t fontId = fontIDs[fontIndex];
  tft.setTextFont(fontId);
  tft.drawString(previewText, 10, yPos);
  
  yPos += 40;
  tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
  tft.setTextFont(2);
  tft.drawString("Size Example:", 10, yPos, 2);
  
  yPos += 20;
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(fontId);
  tft.setTextSize(1);
  tft.drawString("Size 1x", 10, yPos);
  
  yPos += 25;
  tft.setTextSize(2);
  tft.drawString("Size 2x", 10, yPos);
  
  yPos += 35;
  tft.setTextSize(1);
  
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextFont(2);
  tft.drawString("Character Set:", 10, yPos, 2);
  
  yPos += 20;
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(fontId);
  
  String charSet = " !\"#$%&'()*+,-./";
  tft.drawString(charSet, 10, yPos);
  
  yPos += 20;
  charSet = "0123456789:;<=>?@";
  tft.drawString(charSet, 10, yPos);
  
  yPos += 20;
  charSet = "ABCDEFGHIJKLMNO";
  tft.drawString(charSet, 10, yPos);
  
  yPos += 20;
  charSet = "PQRSTUVWXYZ[\\]^_";
  tft.drawString(charSet, 10, yPos);
  
  yPos += 20;
  charSet = "abcdefghijklmno";
  tft.drawString(charSet, 10, yPos);
  
  yPos += 20;
  charSet = "pqrstuvwxyz{|}~";
  tft.drawString(charSet, 10, yPos);
  
  int footerY = SCREEN_HEIGHT - 25;
  tft.fillRect(0, footerY, SCREEN_WIDTH, 25, TFT_NAVY);
  tft.setTextColor(TFT_SILVER, TFT_NAVY);
  tft.setTextDatum(TC_DATUM);
  tft.setTextFont(1);
  tft.drawString("ESP32-2432S028R CYD Font Preview", SCREEN_WIDTH / 2, footerY + 8);
  
  Serial.print("Showing font: ");
  Serial.println(fontNames[fontIndex]);
}
