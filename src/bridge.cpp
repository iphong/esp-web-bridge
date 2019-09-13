#include "bridge.h"
#include "http.h"

const SerialConfig SERIAL_MODES[] = {
  SERIAL_5N1, SERIAL_6N1, SERIAL_7N1, SERIAL_8N1,
  SERIAL_5N2, SERIAL_6N2, SERIAL_7N2, SERIAL_8N2,
  SERIAL_5E1, SERIAL_6E1, SERIAL_7E1, SERIAL_8E1,
  SERIAL_5E2, SERIAL_6E2, SERIAL_7E2, SERIAL_8E2,
  SERIAL_5O1, SERIAL_6O1, SERIAL_7O1, SERIAL_8O1,
  SERIAL_5O2, SERIAL_6O2, SERIAL_7O2, SERIAL_8O2
};

static const char * SERIAL_MODES_STR[] = {
  "5N1" , "6N1" , "7N1" , "8N1" ,
  "5N2" , "6N2" , "7N2" , "8N2" ,
  "5E1" , "6E1" , "7E1" , "8E1" ,
  "5E2" , "6E2" , "7E2" , "8E2" ,
  "5O1" , "6O1" , "7O1" , "8O1" ,
  "5O2" , "6O2" , "7O2" , "8O2"
};

static const char * SERIAL_BAUDRATES[] = {
  "4800", "9600", "19200", "38400",
  "57600", "74880", "115200", "230400"
};


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

int lookup(const char * value, const char ** array, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (array[i] == value) return i;
  }
  return -1;
}

bool uart_active;
WiFiClient clients[4];
uint8_t buf[512];
Ticker hb([]() {  }, 500, MILLIS);

void debug(String s) {
  ws.broadcastTXT(s);
}

void uart_init() {
  uart_close();
  Serial.begin(
    cfg.bridge_baud,
    SERIAL_MODES[lookup(cfg.bridge_mode, SERIAL_MODES_STR, ARRAYLEN(SERIAL_MODES_STR))]
  );
  if (cfg.bridge_swap) Serial.swap();
  Serial.setDebugOutput(false);
  debug((String)"\nUART: " + cfg.bridge_mode + " "  + cfg.bridge_baud + " - SWAP:" + cfg.bridge_swap + "\n");
  uart_active = true;
}

void uart_close() {
  if (uart_active) {
    Serial.flush();
    Serial.end();
    uart_active = false;
  }
}

void bridge_init() {
  ws.onEvent(ws_handler);
  ws.begin();
  ss.setNoDelay(true);
  ss.begin();
  hb.start();
  uart_init();
}

void bridge_loop() {
  ws.loop();
  hb.update();
  for (int i = 0; i < 4; i++) {
    if (!clients[i]) { // equivalent to !serverClients[i].connected()
      clients[i] = ss.available();
      break;
    }
  }
  for (int i = 0; i < 4; i++) {
    if (clients[i].available()) {
      size_t b = 0;
      while (clients[i].available()) {
        buf[b++] = clients[i].read();
      }
      ws.broadcastTXT(buf, b);
      if (Serial.availableForWrite()) {
        Serial.write(buf, b);
      }
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
}

void ws_handler(uint8_t num, WStype_t type, uint8_t * payload, size_t len) {
  switch (type) {
    case WStype_TEXT:
      if (uart_active) {
        for (int i = 0; i < 4; i++) {
          if (clients[i] && clients[i].availableForWrite()) {
            clients[i].write(payload, len);
          }
        }
      }
      break;
    case WStype_BIN:
      if (uart_active) {
        Serial.printf("%s", payload);
      }
      break;
    case WStype_CONNECTED:
      send_info();
      // uart_init();
      break;
    case WStype_DISCONNECTED:
      // uart_close();
      break;
      default: {}
  }
}
