#include "main.h"

void wifi_check() {
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    digitalWrite(cfg.led_pin, HIGH);
    delay(300);
    digitalWrite(cfg.led_pin, LOW);
    delay(300);
  }
}

void wifi_setup() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(wifi_station_get_hostname(), "");
  if (cfg.sta_ssid != "") {
    WiFi.begin(cfg.sta_ssid, cfg.sta_pass);
  }
  wifi_check();
}

void wifi_handle_http_req() {

}
