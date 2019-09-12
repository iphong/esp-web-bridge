#include "uart.h"
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

int lookup(const char * value, const char ** array, size_t size) {
  for (size_t i = 0; i < size; i++) {
    if (array[i] == value) return i;
  }
  return -1;
}

bool serial_active;

void debug(String s) {
  ws.broadcastTXT(s);
}

void serial_setup() {
  serial_close();
  avrprog.setReset(true);

  Serial.begin(
    cfg.serial_baud,
    SERIAL_MODES[lookup(cfg.serial_mode, SERIAL_MODES_STR, ARRAYLEN(SERIAL_MODES_STR))]
  );

  while (Serial.available()) Serial.read();
  if (cfg.serial_swap) Serial.swap();
  Serial.setDebugOutput(false);
  debug((String)"\nSERIAL: " + cfg.serial_mode + " "  + cfg.serial_baud + " - SWAP:" + cfg.serial_swap + "\n");
  serial_active = true;
  avrprog.setReset(false);
}

void serial_close() {
  if (serial_active) {
    Serial.flush();
    Serial.end();
    serial_active = false;
  }
}

void serial_loop() {

}
