#include <Arduino.h>
#include <SPI.h>
#include <FS.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266AVRISP.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

#ifndef MAIN_H_
#define MAIN_H_

struct Config {
  char * hostname = (char *)"esp";

  char * sta_ssid = (char *)"";
  char * sta_pass = (char *)"";

  int pgm_port = 23;
  int http_port = 80;
  int ws_port = 88;

  int led_pin = 2;
  int reset_pin = 5;

  bool serial_swap     = false;
  bool serial_debug    = false;
  String serial_mode   = "8N1";
  uint32_t serial_baud = 115200;
};
extern Config cfg;
extern ESP8266AVRISP avrprog;
extern ESP8266WebServer http;
extern WebSocketsServer ws;
extern bool serial_active;

void debug(String msg);


#endif // MAIN_H_
