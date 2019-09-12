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
  if (strlen(cfg.sta_ssid) != 0) {
    WiFi.begin(cfg.sta_ssid, cfg.sta_pass);
  }
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();
  wifi_check();
  display.println("Connected.");
  display.display();
}

void wifi_handle_http_req() {

}
