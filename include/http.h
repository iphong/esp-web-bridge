#include <Arduino.h>
#include "main.h"

#ifndef http_h_
#define http_h_

void http_setup();

void http_loop();

void send_json(char *);

#endif
