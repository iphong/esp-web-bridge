#include <Arduino.h>
#include "main.h"

#ifndef ws_h_
#define ws_h_

void ws_handler(uint8_t num, WStype_t type, uint8_t * payload, size_t len);

#endif
