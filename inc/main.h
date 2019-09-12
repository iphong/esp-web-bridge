#include <Arduino.h>
#include <SPI.h>
#include <Ticker.h>
#include <Wire.h>
#include <FS.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266AVRISP.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef MAIN_H_
#define MAIN_H_

#define ARRAYLEN(x) (sizeof(x) / sizeof((x)[0]))
#define ARRAYEND(x) (&(x)[ARRAYLEN(x)])

#define LOOKUP_TABLE_ENTRY(name) { name, ARRAYLEN(name) }
#define LOOKUP_TABLE_ARG(name) name, ARRAYLEN(name)

struct Config {
  const char * hostname = "esp";

  const char * sta_ssid = "Tu";
  const char * sta_pass = "88888888";

  int led_pin = 2;
  int reset_pin = 16;

  bool serial_swap     = false;
  const char * serial_mode   = "8N1";
  uint32_t serial_baud = 115200;
};

extern Config cfg;
extern ESP8266AVRISP avrprog;
extern ESP8266WebServer http;
extern WebSocketsServer ws;
extern bool serial_active;
extern Adafruit_SSD1306 display;

extern uint16_t dur;

void debug(String msg);


#endif // MAIN_H_
