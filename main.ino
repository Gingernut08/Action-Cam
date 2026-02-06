#include "esp_camera.h"
#include "FS.h"
#include "SD_MMC.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define RECORD_SWITCH_GPIO 1   // switch to GND (INPUT_PULLUP)
#define RECORD_LED_GPIO    3   // MOSFET gate (HIGH = LED ON)

bool recording = false;
int frameNumber = 0;

void setup() {
  // Serial disabled: GPIO 1 & 3 used
  // Serial.begin(115200);

  pinMode(RECORD_SWITCH_GPIO, INPUT_PULLUP);

  pinMode(RECORD_LED_GPIO, OUTPUT);
  digitalWrite(RECORD_LED_GPIO, LOW);   // MOSFET OFF at boot

  // Camera configuration
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;   config.ledc_timer   = LEDC_TIMER_0;   config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;      config.pin_d2       = Y4_GPIO_NUM;    config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;      config.pin_d5       = Y7_GPIO_NUM;    config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;      config.pin_xclk     = XCLK_GPIO_NUM;  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;   config.pin_href     = HREF_GPIO_NUM;  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;    config.pin_pwdn     = PWDN_GPIO_NUM;  config.pin_reset    = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size   = FRAMESIZE_QVGA; // 320x240
  config.jpeg_quality = 12;
  config.fb_count     = 2;

  esp_camera_init(&config);
  SD_MMC.begin();
}

void loop() {
  bool switchPressed = (digitalRead(RECORD_SWITCH_GPIO) == LOW);

  if (switchPressed && !recording) {
    recording = true;
    frameNumber = 0;
    digitalWrite(RECORD_LED_GPIO, HIGH); // MOSFET ON
  }

  if (!switchPressed && recording) {
    recording = false;
    digitalWrite(RECORD_LED_GPIO, LOW);  // MOSFET OFF
  }

  if (!recording) {
    delay(50);
    return;
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return;

  char path[32];
  sprintf(path, "/vid_%05d.jpg", frameNumber++);

  File file = SD_MMC.open(path, FILE_WRITE);
  if (file) {
    file.write(fb->buf, fb->len);
    file.close();
  }

  esp_camera_fb_return(fb);

  delay(80); // ~12 FPS
}
