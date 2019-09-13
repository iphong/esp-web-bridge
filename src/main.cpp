#include "main.h"
#include "bridge.h"
#include "wifi.h"
#include "http.h"
#include "ui.h"

Config cfg;
ESP8266AVRISP avrprog(23, cfg.reset_pin);
ESP8266WebServer http(80);
WebSocketsServer ws(81);
WiFiServer ss(82);
Adafruit_SSD1306 display(128, 64, &Wire, -1, 800000UL, 800000UL);

uint32_t tmr;
uint16_t dur;

void setup() {

  uart_init();
  gpio_init();
  ui_init();
  wifi_init();
  bridge_init();

  SPIFFS.begin();

  MDNS.begin(cfg.hostname);
  MDNS.addService("avrisp", "tcp", 23);
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ota", "tcp", 8266);
  MDNS.addService("ws", "tcp", 81);

  ArduinoOTA.setHostname(cfg.hostname);
  ArduinoOTA.begin(true);

  avrprog.begin();
  avrprog.setReset(false);

  http.on("/pgm/sync", [](){
    if (http.method() == HTTP_POST) {
        Serial.println("sync post");
        http.send(204);
    }
    if (http.method() == HTTP_GET) {
        Serial.println("sync check");
        http.send(200, "text/plain", "SYNC at 115200 baud: bootloader v2.1");
    }
  });
  http.on("/pgm/upload", HTTP_GET, [](){
    Serial.println((String)"Upload check");
    http.send(200);
  });
  http.on("/pgm/upload", HTTP_POST, [](){
    Serial.println((String)"Upload post");
    Serial.println(http.arg("plain"));
    http.send(204, "text/plain", ".");
  });

  http.on("/wifi", wifi_handler);
  http.on("/config", config_handler);

  http.on("/files", HTTP_GET, fs_list_handler);
  http.on("/files", HTTP_POST, send_ok, fs_upload_handler);
  http.onNotFound(fs_handler);
  // http.on("/command/reset", reset_handler);
  // http.on("/command/detect_baudrate", baudrate_detect_handler);

  http.begin();
}

void loop() {
  tmr = millis();
  wifi_check();

  MDNS.update();
  ArduinoOTA.handle();
  http.handleClient();

  switch (avrprog.update()) {
    case AVRISP_STATE_PENDING:
    case AVRISP_STATE_ACTIVE:
      avrprog.serve();
      break;
    case AVRISP_STATE_IDLE:
      bridge_loop();
      ui_loop();
      break;
  }
  dur = millis() - tmr;
}
