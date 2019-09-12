#include "main.h"
#include "uart.h"
#include "wifi.h"
#include "http.h"
#include "ws.h"
#include "ui.h"

Config cfg;
ESP8266AVRISP avrprog(23, cfg.reset_pin);
ESP8266WebServer http(80);
WebSocketsServer ws(81);
WiFiServer ss(82);
Adafruit_SSD1306 display(128, 64, &Wire, -1, 800000UL, 800000UL);

uint32_t tmr;
uint16_t dur;

Ticker hb([]() {  }, 500, MILLIS);

void setup() {

  avrprog.setReset(true);
  serial_setup();

  SPIFFS.begin();

  pinMode(cfg.led_pin, OUTPUT);
  digitalWrite(cfg.led_pin, LOW);

  ui_init();
  wifi_setup();

  MDNS.begin(cfg.hostname);
  MDNS.addService("avrisp", "tcp", 23);
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ota", "tcp", 8266);
  MDNS.addService("ws", "tcp", 81);

  ArduinoOTA.setHostname(cfg.hostname);
  ArduinoOTA.begin(true);

  ws.onEvent(ws_handler);
  ws.begin();

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
  ss.setNoDelay(true);
  ss.begin();

  hb.start();
}

WiFiClient clients[4];
uint8_t buf[512];
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
      hb.update();
      serial_loop();


      for (int i = 0; i < 4; i++) {
        if (!clients[i]) { // equivalent to !serverClients[i].connected()
          clients[i] = ss.available();
          break;
        }
      }

      for (int i = 0; i < 4; i++) {
        while (clients[i].available() && Serial.availableForWrite()) {
          Serial.write(clients[i].read());
        }
      }
      if (Serial.available()) {
        size_t b = 0;
        while (Serial.available() && b < 512) {
          buf[b++] = Serial.read();
        }
        ws.broadcastBIN(buf, b);
        for (int i = 0; i < 4; i++) {
          if (clients[i] && clients[i].availableForWrite()) {
            clients[i].write(buf, b);
          }
        }
      }

      ui_loop();
      break;
  }
  dur = millis() - tmr;
}
