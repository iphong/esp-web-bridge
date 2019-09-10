#include "http.h"
#include "uart.h"

File fs_upload_file;

void send_ok() {
  http.sendHeader("Access-Control-Allow-Origin", "*");
  http.send(200, "application/json", "\"OK\"");
}

void send_404() {
  http.sendHeader("Access-Control-Allow-Origin", "*");
  http.send(404, "text/plain", "FileNotFound");
}

void send_json(char * data) {
  http.sendHeader("Access-Control-Allow-Origin", "*");
  http.send(200, "application/json", data);
}

String get_content_type(String filename) {
  if (http.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool fs_read_handler() {
  String path = http.uri();
  if (path.endsWith("/")) {
    path += "index.html";
  }
  String contentType = get_content_type(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = SPIFFS.open(path, "r");
    http.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void fs_upload_handler() {
  HTTPUpload& upload = http.upload();
  if (upload.status == UPLOAD_FILE_START) {
    debug((String)"\nUpload start -> " + upload.filename + ":");
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    fs_upload_file = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    debug(".");
    if (fs_upload_file) {
      fs_upload_file.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fs_upload_file) {
      fs_upload_file.close();
    }
    debug("\n");
  }
}

void fs_delete_handler() {
  if (http.args() == 0) {
    return http.send(500, "text/plain", "BAD ARGS");
  }
  String path = http.arg(0);
  if (path == "/") {
    return http.send(500, "text/plain", "BAD PATH");
  }
  if (!SPIFFS.exists(path)) {
    return http.send(404, "text/plain", "FileNotFound");
  }
  SPIFFS.remove(path);
  send_ok();
  path = String();
}

void fs_create_handler() {
  if (http.args() == 0) {
    return http.send(500, "text/plain", "BAD ARGS");
  }
  String path = http.arg(0);
  if (path == "/") {
    return http.send(500, "text/plain", "BAD PATH");
  }
  if (SPIFFS.exists(path)) {
    return http.send(500, "text/plain", "FILE EXISTS");
  }
  File file = SPIFFS.open(path, "w");
  if (file) {
    file.close();
  } else {
    return http.send(500, "text/plain", "CREATE FAILED");
  }
  send_ok();
  path = String();
}

void fs_list_handler() {
  if (!http.hasArg("dir")) {
    http.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = http.arg("dir");
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") {
      output += ',';
    }
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  http.send(200, "text/json", output);
}

bool save_cfg_int(uint32_t *addr, String arg) {
  if (http.hasArg(arg)) {
    *addr = atoi(http.arg(arg).c_str());
    return true;
  }
  return false;
}
bool save_cfg_int(int *addr, String arg) {
  if (http.hasArg(arg)) {
    *addr = atoi(http.arg(arg).c_str());
    return true;
  }
  return false;
}
bool save_cfg_bool(bool *addr, String arg) {
  if (http.hasArg(arg)) {
    *addr = http.arg(arg) == "0" ? false : true;
    return true;
  }
  return false;
}

void config_handler() {
  switch (http.method()) {
    case HTTP_POST: {
      bool serial = false;
      save_cfg_int(&cfg.reset_pin, "reset_pin");
      serial |= save_cfg_int(&cfg.serial_baud, "serial_baud");
      serial |= save_cfg_bool(&cfg.serial_swap, "serial_swap");
      if (http.hasArg("serial_mode")) {
        cfg.serial_mode = http.arg("serial_mode");
        serial = true;
      }
      if (serial_active) {
        serial_setup();
      }
      send_ok();
      break;
    }
    case HTTP_GET: {
      char res[1024];
      sprintf(res,
        "{"
          "\"serial_mode\": \"%s\","
          "\"serial_baud\": %u,"
          "\"serial_swap\": %u,"
          "\"serial_debug\": %u,"
          "\"reset_pin\": %u,"
          "\"hostname\": \"%s\""
        "}",
        cfg.serial_mode.c_str(),
        cfg.serial_baud,
        cfg.serial_swap,
        cfg.serial_debug,
        cfg.reset_pin,
        cfg.hostname
      );
      send_json((char *)res);
      break;
    }
    default: {}
  }
}

void reset_handler() {
  avrprog.setReset(true);
  avrprog.setReset(false);
  send_ok();
}

void baudrate_detect_handler() {
  uint32_t baud = Serial.detectBaudrate(10000);
  http.sendHeader("Access-Control-Allow-Origin", "*");
  http.send(200, "application/json", (String)baud);
}

void http_setup() {

  SPIFFS.begin();

  http.on("/fs", HTTP_GET, fs_list_handler);
  http.on("/fs", HTTP_PUT, fs_create_handler);
  http.on("/fs", HTTP_DELETE, fs_delete_handler);
  http.on("/fs", HTTP_POST, send_ok, fs_upload_handler);

  http.on("/config", config_handler);
  http.on("/command/reset", reset_handler);
  http.on("/command/detect_baudrate", baudrate_detect_handler);

  http.onNotFound(fs_read_handler);

  http.begin();
}

void http_loop() {
  http.handleClient();
}
