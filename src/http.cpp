#include "http.h"
#include "uart.h"
#include "wifi.h"

File fs_upload_file;

void send_ok() {
  http.send(200, "application/json", "\"OK\"");
}

void send_404() {
  http.send(404, "text/plain", "FileNotFound");
}

void send_json(char * data) {
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

void fs_read_handler() {
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
  } else {
    if (http.uri().endsWith("/"))
      return fs_list_handler();
    send_404();
  }
}

void fs_upload_handler() {
  HTTPUpload& upload = http.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = "/";
    if (http.hasArg("dir")) {
      filename = http.arg("dir");
      if (!filename.endsWith("/")) filename += "/";
      if (!filename.startsWith("/")) filename = "/" + filename;
    }
    filename = filename + upload.filename;
    debug((String)"\nUpload start -> " + filename + ":");
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
  String path = http.uri();
  if (path.endsWith("/")) {
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
  String path = http.uri();
  if (path.endsWith("/")) {
    return http.send(500, "text/plain", "BAD PATH");
  }
  if (SPIFFS.exists(path)) {
    return http.send(500, "text/plain", "FILE EXISTS");
  }
  File file = SPIFFS.open(path, "w");
  if (file) {
    file.write(http.arg("plain").c_str());
    file.close();
  } else {
    return http.send(500, "text/plain", "CREATE FAILED");
  }
  RequestHandler handler();
  send_ok();
  path = String();
}

void fs_update_handler() {
  String path = http.uri();

  if (path.endsWith("/")) {
    return http.send(500, "text/plain", "BAD PATH");
  }
  File file = SPIFFS.open(path, "w");
  if (file) {
    file.write(http.arg("plain").c_str());
    file.close();
  } else {
    return http.send(500, "text/plain", "CREATE FAILED");
  }
  send_ok();
  path = String();
}

void fs_list_handler() {
  String path = "/";
  if (http.hasArg("dir")) {
    path = http.arg("dir");
  }
  if (!path.endsWith("/")) path += "/";
  if (!path.startsWith("/")) path = "/" + path;
  Dir dir = SPIFFS.openDir(path);
  if (http.hasArg("json")) {
    String json = "[";
    while (dir.next()) {
      File entry = dir.openFile("r");
      if (json != "[") {
        json += ',';
      }
      bool isDir = false;
      json += "{\"type\":\"";
      json += (isDir) ? "dir" : "file";
      json += "\",\"name\":\"";
      json += String(entry.name()).substring(1);
      json += "\"}";
      entry.close();
    }
    json += "]";
    http.send(200, "text/json", json);
    json = String();
  } else {
    String html = "<!DOCTYPE html>\n";
    html += "<h1>Index of " + (path.length() == 1 ? path : path.substring(0, path.length() - 1)) + "</h1><ul>";
    while (dir.next()) {
      File entry = dir.openFile("r");
      html += "<li><a href=\"" + String(entry.name()) + "\">" + String(entry.name()).substring(path.length()) + "</a></li>";
      entry.close();
    }
    html += "</ul>";
    http.send(200, "text/html", html);
    html = String();
  }
  path = String();
}

void fs_handler() {
  http.sendHeader("Access-Control-Allow-Origin", "*");
  http.sendHeader("Access-Control-Allow-Methods", "*");
  switch (http.method()) {
    case HTTP_GET:    fs_read_handler();   break;
    case HTTP_PUT:    fs_create_handler(); break;
    case HTTP_PATCH:  fs_update_handler(); break;
    case HTTP_DELETE: fs_delete_handler(); break;
    default: http.send(404);
  }
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
        cfg.serial_mode = http.arg("serial_mode").c_str();
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
          "\"reset_pin\": %u,"
          "\"hostname\": \"%s\""
        "}",
        cfg.serial_mode,
        cfg.serial_baud,
        cfg.serial_swap,
        cfg.reset_pin,
        cfg.hostname
      );
      send_json((char *)res);
      break;
    }
    default: {}
  }
}

void wifi_handler() {
  if (http.hasArg("ssid") && http.hasArg("pass")) {
    cfg.sta_ssid = http.arg("ssid").c_str();
    cfg.sta_pass = http.arg("pass").c_str();
    wifi_setup();
    send_json((char *)"OK");
  } else {
    send_json((char *)"NOT OK");
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
