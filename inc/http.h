#include <Arduino.h>
#include "main.h"

#ifndef http_h_
#define http_h_
#include "http.h"
#include "uart.h"
#include "wifi.h"

void send_ok();

void send_404();

void send_json(char *);

String get_content_type(String filename);

void fs_read_handler();

void fs_upload_handler();

void fs_delete_handler();

void fs_create_handler();

void fs_list_handler();

void fs_handler();

void config_handler();

void wifi_handler();

void reset_handler();

void baudrate_detect_handler();

#endif
