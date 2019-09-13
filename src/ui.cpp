#include "ui.h"

void ui_init() {
  pinMode(A0, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();
}

void ui_loop() {
  uint16_t fps = (double)1000 / dur;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(WiFi.localIP());
  display.println(WiFi.softAPIP());
  display.println(analogRead(A0));
  display.setCursor(0, 56);
  display.print(fps);
  display.print(" fps");
  display.display();
}
