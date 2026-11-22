
#include "esp_camera.h"
#include "img_converters.h"
#include <WiFi.h>
#include "esp_http_server.h"

const char* WIFI_SSID = "Rao sahab";
const char* WIFI_PASS = "Vivek@2006";

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

static esp_err_t streamHandler(httpd_req_t* req) {
  esp_err_t res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=frame");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  char part[64];
  while (res == ESP_OK) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) { res = ESP_FAIL; break; }

    uint8_t* jpg = NULL;
    size_t jpgLen = 0;
    bool ok = frame2jpg(fb, 70, &jpg, &jpgLen);
    esp_camera_fb_return(fb);
    if (!ok) { res = ESP_FAIL; break; }

    size_t hl = snprintf(part, 64, "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", (unsigned)jpgLen);
    res = httpd_resp_send_chunk(req, "\r\n--frame\r\n", 11);
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, part, hl);
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char*)jpg, jpgLen);
    free(jpg);
  }
  return res;
}

void startServer() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 81;
  config.ctrl_port = 32769;
  httpd_uri_t uri = {"/stream", HTTP_GET, streamHandler, NULL};
  httpd_handle_t h = NULL;
  if (httpd_start(&h, &config) == ESP_OK) httpd_register_uri_handler(h, &uri);
}

void setup() {
  Serial.begin(115200);
  camera_config_t c;
  c.ledc_channel = LEDC_CHANNEL_0; c.ledc_timer = LEDC_TIMER_0;
  c.pin_d0 = Y2_GPIO_NUM; c.pin_d1 = Y3_GPIO_NUM; c.pin_d2 = Y4_GPIO_NUM; c.pin_d3 = Y5_GPIO_NUM;
  c.pin_d4 = Y6_GPIO_NUM; c.pin_d5 = Y7_GPIO_NUM; c.pin_d6 = Y8_GPIO_NUM; c.pin_d7 = Y9_GPIO_NUM;
  c.pin_xclk = XCLK_GPIO_NUM; c.pin_pclk = PCLK_GPIO_NUM;
  c.pin_vsync = VSYNC_GPIO_NUM; c.pin_href = HREF_GPIO_NUM;
  c.pin_sccb_sda = SIOD_GPIO_NUM; c.pin_sccb_scl = SIOC_GPIO_NUM;
  c.pin_pwdn = PWDN_GPIO_NUM; c.pin_reset = RESET_GPIO_NUM;
  c.xclk_freq_hz = 20000000;
  c.pixel_format = PIXFORMAT_RGB565;     
  c.frame_size = FRAMESIZE_QVGA;          
  c.jpeg_quality = 12;
  c.fb_count = 2;
  c.fb_location = CAMERA_FB_IN_PSRAM;
  c.grab_mode = CAMERA_GRAB_LATEST;
  if (esp_camera_init(&c) != ESP_OK) { Serial.println("Camera init failed"); return; }

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(400); Serial.print("."); }
  startServer();
  Serial.print("\nStream: http://"); Serial.print(WiFi.localIP()); Serial.println(":81/stream");
}

void loop() { delay(10000); }
