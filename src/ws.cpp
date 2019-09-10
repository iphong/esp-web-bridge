#include "ws.h"
#include "uart.h"

char tmp[20];
const char * const RST_REASONS[] = {
  "REASON_DEFAULT_RST",
  "REASON_WDT_RST",
  "REASON_EXCEPTION_RST",
  "REASON_SOFT_WDT_RST",
  "REASON_SOFT_RESTART",
  "REASON_DEEP_SLEEP_AWAKE",
  "REASON_EXT_SYS_RST"
};

const char * const FLASH_SIZE_MAP_NAMES[] = {
  "4M_MAP_256_256",
  "2M",
  "8M_MAP_512_512",
  "16M_MAP_512_512",
  "32M_MAP_512_512",
  "16M_MAP_1024_1024",
  "32M_MAP_1024_1024"
};

const char * const OP_MODE_NAMES[] {
  "NULL",
  "STATION",
  "SOFTAP",
  "STATIONAP"
};

const char * const AUTH_MODE_NAMES[] {
  "OPEN",
  "WEP",
  "WPA_PSK",
  "WPA2_PSK",
  "WPA_WPA2_PSK",
  "MAX"
};

const char * const PHY_MODE_NAMES[] {
  "",
  "B",
  "G",
  "N"
};

const char * const EVENT_NAMES[] {
  "EVENT_STAMODE_CONNECTED",
  "EVENT_STAMODE_DISCONNECTED",
  "EVENT_STAMODE_AUTHMODE_CHANGE",
  "EVENT_STAMODE_GOT_IP",
  "EVENT_SOFTAPMODE_STACONNECTED",
  "EVENT_SOFTAPMODE_STADISCONNECTED",
  "EVENT_MAX"
};

const char * const EVENT_REASONS[] {
  "",
  "REASON_UNSPECIFIED",
  "REASON_AUTH_EXPIRE",
  "REASON_AUTH_LEAVE",
  "REASON_ASSOC_EXPIRE",
  "REASON_ASSOC_TOOMANY",
  "REASON_NOT_AUTHED",
  "REASON_NOT_ASSOCED",
  "REASON_ASSOC_LEAVE",
  "REASON_ASSOC_NOT_AUTHED",
  "REASON_DISASSOC_PWRCAP_BAD",
  "REASON_DISASSOC_SUPCHAN_BAD",
  "REASON_IE_INVALID",
  "REASON_MIC_FAILURE",
  "REASON_4WAY_HANDSHAKE_TIMEOUT",
  "REASON_GROUP_KEY_UPDATE_TIMEOUT",
  "REASON_IE_IN_4WAY_DIFFERS",
  "REASON_GROUP_CIPHER_INVALID",
  "REASON_PAIRWISE_CIPHER_INVALID",
  "REASON_AKMP_INVALID",
  "REASON_UNSUPP_RSN_IE_VERSION",
  "REASON_INVALID_RSN_IE_CAP",
  "REASON_802_1X_AUTH_FAILED",
  "REASON_CIPHER_SUITE_REJECTED",
};

const char * const EVENT_REASONS_200[] {
  "REASON_BEACON_TIMEOUT",
  "REASON_NO_AP_FOUND"
};

String ip2str(IPAddress ip) {
  return (String)ip[0] + "." + ip[1] + "." + ip[2] + "." + ip[3];
}

void send_info() {
  softap_config ap_cfg;
  station_config sta_cfg;
  wifi_station_get_config(&sta_cfg);
  wifi_station_get_ap_info(&sta_cfg);
  wifi_softap_get_config(&ap_cfg);

  debug("\n[ SYSTEM ]\n");
  debug((String)"chip_id: 0x" + itoa(system_get_chip_id(), tmp, 16) + "\n");
  debug((String)"sdk_version: " + system_get_sdk_version() + "\n");
  debug((String)"boot_version: " + itoa(system_get_boot_version(), tmp, 10) + "\n");
  debug((String)"userbin_addr: 0x" + itoa(system_get_userbin_addr(), tmp, 16) + "\n");
  debug((String)"name: " + wifi_station_get_hostname() + "\n");

  debug((String)"cpu_freq: " + itoa(system_get_cpu_freq(), tmp, 10) + "Mhz \n");
  debug((String)"flash_size: " + FLASH_SIZE_MAP_NAMES[system_get_flash_size_map()] + "\n");

  debug("\n\n[ ESP OTA ]\n");
  debug((String)"host: " + ArduinoOTA.getHostname() + ".local\n");


  debug("\n\n[ AVR ISP ]\n");
  debug((String)"# avrdude -carduino -pm328p -Pnet:" + ip2str(WiFi.localIP()) + ":23 -t (or -U)\n");
  debug((String)"# avrdude -carduino -pm328p -Pnet:" + ArduinoOTA.getHostname() + ".local:23 -t (or -U)\n");

  debug("\n\n[ WIFI ]\n");
  debug((String)"opmode: " + OP_MODE_NAMES[wifi_get_opmode()] + "\n");
  debug((String)"phy_mode: " + PHY_MODE_NAMES[wifi_get_phy_mode()] + "\n");
  // debug((String)"opmode_default: " + wifi_get_opmode_default() + "\n");
  // debug((String)"broadcast_if: " + wifi_get_broadcast_if() + "\n");
  // debug((String)"channel: " + wifi_get_channel() + "\n");

  debug("\n\n[ WIFI STATION ]\n");

  debug("ssid: ");
  debug((char *)sta_cfg.ssid);
  debug("\n");

  debug("pass: ");
  debug((char *)sta_cfg.password);
  debug("\n\n");

  debug((String)"auto_connect: " + wifi_station_get_auto_connect() + "\n");
  debug((String)"rssi: " + wifi_station_get_rssi() + "\n");

  debug("\n\n[ WIFI SOFTAP ]\n");

  debug("ssid: ");
  debug((char *)ap_cfg.ssid);
  debug("\n\n");

  debug("pass: ");
  debug((char *)ap_cfg.password);
  debug("\n");

  debug((String)"channel: " + itoa(ap_cfg.channel, tmp, 10) + "\n");
  debug((String)"auth_mode: " + AUTH_MODE_NAMES[ap_cfg.authmode] + "\n");
  debug((String)"ssid_hidden: " + ap_cfg.ssid_hidden + "\n");
  debug((String)"max_connection: " + itoa(ap_cfg.max_connection, tmp, 10) + "\n");
  debug((String)"beacon_interval: " + itoa(ap_cfg.beacon_interval, tmp, 10) + "ms\n");

  debug("\n\n--------------------------------------\n");
}

void ws_handler(uint8_t num, WStype_t type, uint8_t * payload, size_t len) {
  switch (type) {
    case WStype_TEXT:
      if (serial_active) {
        Serial.printf("%s", payload);
      }
      break;
    case WStype_BIN:
      if (serial_active) {
        for (size_t i = 0; i < len; i++)
          Serial.write(payload[i]);
      }
      break;
    case WStype_CONNECTED:
      send_info();
      serial_setup();
      break;
    case WStype_DISCONNECTED:
      serial_close();
      break;
    default: {}
  }
}
