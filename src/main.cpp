#include "main.h"
#include "uart.h"
#include "wifi.h"
#include "http.h"
#include "ws.h"
#include "ui.h"

Config cfg;
ESP8266AVRISP avrprog(cfg.pgm_port, cfg.reset_pin);
ESP8266WebServer http(cfg.http_port);
WebSocketsServer ws(cfg.ws_port);
Adafruit_SSD1306 display(128, 64, &Wire, -1, 1000000UL, 100000UL);

uint32_t tmr;
uint16_t dur;

void setup() {

  avrprog.setReset(true);

  SPIFFS.begin();

  pinMode(cfg.led_pin, OUTPUT);
  digitalWrite(cfg.led_pin, LOW);

  ui_init();
  wifi_setup();

  MDNS.begin(cfg.hostname);
  MDNS.addService("avrisp", "tcp", cfg.pgm_port);
  MDNS.addService("http", "tcp", cfg.http_port);
  MDNS.addService("ota", "tcp", cfg.ota_port);
  MDNS.addService("ws", "tcp", cfg.ws_port);

  ArduinoOTA.setHostname(cfg.hostname);
  ArduinoOTA.begin(true);

  ws.onEvent(ws_handler);
  ws.begin();

  avrprog.begin();
  avrprog.setReset(false);

  // http.onFileUpload(fs_upload_handler);

  http.on("/wifi", wifi_handler);
  http.on("/files", fs_list_handler);
  http.on("/config", config_handler);

  http.on("/", HTTP_POST, fs_handler, fs_upload_handler);
  http.onNotFound(fs_handler);
  http.serveStatic("/", SPIFFS, "/");
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
      ws.loop();
      serial_loop();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(WiFi.localIP());
      display.println(WiFi.softAPIP());
      display.setCursor(0, 56);
      display.println(dur);
      ui_loop();
      break;
  }
  dur = millis() - tmr;
}
