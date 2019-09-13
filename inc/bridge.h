#include <Arduino.h>
#include "main.h"

#ifndef bridge_h_
#define bridge_h_

void ws_handler(uint8_t num, WStype_t type, uint8_t * payload, size_t len);

void uart_init();

void uart_close();

void bridge_init();

void bridge_loop();

#endif
