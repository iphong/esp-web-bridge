#include "main.h"
#include "uart.h"
#include "wifi.h"
#include "http.h"
#include "ws.h"

Config cfg;
ESP8266AVRISP avrprog(cfg.pgm_port, cfg.reset_pin);
ESP8266WebServer http(cfg.http_port);
WebSocketsServer ws(cfg.ws_port);

void setup() {
  pinMode(cfg.led_pin, OUTPUT);
  digitalWrite(cfg.led_pin, LOW);
  avrprog.setReset(true);

  wifi_setup();

  MDNS.begin(cfg.hostname);
  MDNS.addService("avrisp", "tcp", cfg.pgm_port);
  MDNS.addService("http", "tcp", cfg.http_port);

  ArduinoOTA.setHostname(cfg.hostname);
  ArduinoOTA.begin(true);

  ws.onEvent(ws_handler);
  ws.begin();

  http_setup();

  avrprog.begin();
  avrprog.setReset(false);
}

void loop() {
  wifi_check();

  MDNS.update();
  ArduinoOTA.handle();
  http_loop();

  switch (avrprog.update()) {
    case AVRISP_STATE_PENDING:
    case AVRISP_STATE_ACTIVE:
      avrprog.serve();
      break;
    case AVRISP_STATE_IDLE:
      ws.loop();
      serial_loop();
      break;
  }
}
