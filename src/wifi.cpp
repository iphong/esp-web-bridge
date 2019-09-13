#include "main.h"

void wifi_check() {
  while (WiFi.status() != WL_CONNECTED) {
    display.print(".");
    display.display();
  }
}

void wifi_init() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(wifi_station_get_hostname(), "");
  if (strlen(cfg.sta_ssid) != 0) {
    WiFi.begin(cfg.sta_ssid, cfg.sta_pass);
  }
  display.setCursor(0, 0);
  display.println("WiFi started.");
  display.print("WiFi connecting");
  display.display();
  wifi_check();
  display.println("WiFi connected.");
  display.display();
}

void wifi_handle_http_req() {

}
