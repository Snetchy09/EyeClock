#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <WiFi.h>

#define TFT_SCLK 9
#define TFT_MOSI 10
#define TFT_RST  8
#define TFT_DC   6
#define TFT_CS   7
#define TFT_BL   21

const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

class MyST7789 : public Adafruit_ST7789 {
  public:
    MyST7789(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk, int8_t rst)
      : Adafruit_ST7789(cs, dc, mosi, sclk, rst) {}
    void setOffsets(uint8_t col, uint8_t row) {
      _colstart = _colstart2 = col;
      _rowstart = _rowstart2 = row;
    }
};

MyST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

int gazeX = 0;
int targetGazeX = 0;
bool blink = false;
unsigned long nextGaze = 0;
unsigned long nextBlink = 0;
unsigned long blinkStart = 0;

void connectWiFi() {
  if (String(WIFI_SSID) == "YOUR_WIFI_NAME") return;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(250);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed");
  }
}

void drawEye(int x, int y, int openness, int pupilOffset) {
  int h = map(openness, 0, 100, 3, 42);
  tft.fillRoundRect(x - 28, y - h / 2, 56, h, 14, ST77XX_WHITE);
  if (openness > 12) {
    int pupilY = y;
    int pupilX = x + pupilOffset;
    tft.fillCircle(pupilX, pupilY, 9, ST77XX_BLACK);
    tft.fillCircle(pupilX - 3, pupilY - 3, 2, ST77XX_WHITE);
  }
}

void drawEyes() {
  tft.fillScreen(ST77XX_BLACK);
  int openness = blink ? 0 : 100;
  drawEye(78, 38, openness, gazeX);
  drawEye(206, 38, openness, gazeX);
}

void updateEyes() {
  if (millis() >= nextGaze) {
    targetGazeX = random(-10, 11);
    nextGaze = millis() + random(1200, 3000);
  }
  if (gazeX < targetGazeX) gazeX++;
  if (gazeX > targetGazeX) gazeX--;
  if (!blink && millis() >= nextBlink) {
    blink = true;
    blinkStart = millis();
  }
  if (blink && millis() - blinkStart > 180) {
    blink = false;
    nextBlink = millis() + random(2500, 6000);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, LOW);
  tft.init(76, 284);
  tft.setOffsets(82, 18);
  tft.invertDisplay(false);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(0, 0);
  tft.print("LUCID EYES");
  randomSeed(micros());
  nextGaze = millis() + 1000;
  nextBlink = millis() + 3000;
  connectWiFi();
  Serial.println("TFT Initialized!");
}

void loop() {
  updateEyes();
  drawEyes();
  delay(50);
}