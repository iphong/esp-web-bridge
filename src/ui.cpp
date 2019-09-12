#include "ui.h"

void ui_init() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

void ui_loop() {
  display.display();
}
