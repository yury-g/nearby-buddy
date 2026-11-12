#pragma once

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include "Proximity.h"
#include "src/opentoy/DoodlePixels.h"

inline constexpr char firmwareName[] = "Nearby Buddy";
inline constexpr char firmwareVersion[] = "v0.1.3";

inline void buildTimestamp(char (&text)[20]) {
  constexpr char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
  const char date[] = __DATE__;
  char monthName[] = {date[0], date[1], date[2], 0};
  const char *month = strstr(months, monthName);
  snprintf(text, sizeof text, "%.4s-%02d-%02d %.8s",
           date + 7, month ? int((month - months) / 3) + 1 : 0, atoi(date + 4), __TIME__);
}

class Face {
 public:
  void begin(bool green) {
    green_ = green;
    pinMode(NEOPIXEL_POWER, OUTPUT);
    digitalWrite(NEOPIXEL_POWER, LOW);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    pinMode(TFT_BACKLITE, OUTPUT);
    digitalWrite(TFT_BACKLITE, LOW);
    pinMode(TFT_I2C_POWER, OUTPUT);
    digitalWrite(TFT_I2C_POWER, HIGH);
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(20);
    digitalWrite(TFT_RST, LOW);
    delay(20);
    digitalWrite(TFT_RST, HIGH);
    delay(120);
    SPI.begin(SCK, MISO, MOSI, TFT_CS);
    send(0x01); // SWRESET
    delay(150);
    send(0x11); // SLPOUT
    delay(120);
    send(0x3a, {0x55}); // 16-bit RGB565
    send(0x36, {0xa0}); // Adafruit ST7789 rotation 1
    send(0x21); // INVON
    send(0x13); // NORON
    delay(10);
    clear();
    drawBuildInfo();
    send(0x29); // DISPON
    analogWrite(TFT_BACKLITE, 150);
  }

  bool draw(Mood mood, uint32_t now) {
    if (drawn_ && now - lastTick_ < 33) return false;
    lastTick_ = now;
    if (!drawn_ || mood != lastMood_) {
      changedAt_ = now;
      lastMood_ = mood;
      analogWrite(TFT_BACKLITE, mood == Mood::Sleep ? 44 : 150);
    }
    const uint32_t elapsed = now - changedAt_;
    const FaceState pose = expression(mood, elapsed, now);
    if (drawn_ && !motion_.tick(pose, now, true) && mood == Mood::Sleep) return false;
    if (!drawn_) motion_.reset(pose, now);

    uint8_t pixels[1024];
    uint8_t offset = 0;
    if (mood == Mood::Dance) {
      const uint32_t beat = (elapsed / (green_ ? 280 : 215)) % 4;
      constexpr uint8_t steps[] = {1, 2, 8, 4};
      offset = steps[green_ ? (beat + 1) % 4 : beat];
    }
    drawDoodle(motion_.current, pose.expression, pixels,
               motion_.wiggle(0, now), motion_.wiggle(1, now), motion_.wiggle(2, now), offset);
    if (drawn_ && memcmp(pixels, previous_, sizeof pixels) == 0) return false;
    memcpy(previous_, pixels, sizeof pixels);
    drawn_ = true;
    drawPixels(pixels, green_ ? 0x47e9 : 0x353f);
    return true;
  }

  void drawStatus(int batteryPercent, float feet) {
    if (!statusCanvas_.getBuffer()) return;
    char battery[14];
    if (batteryPercent < 0) snprintf(battery, sizeof battery, "BAT --%%");
    else snprintf(battery, sizeof battery, "BAT %d%%", batteryPercent);

    char distance[14];
    if (feet < 0) snprintf(distance, sizeof distance, "-- ft");
    else if (feet < 1) snprintf(distance, sizeof distance, "~<1 ft");
    else if (feet > 12) snprintf(distance, sizeof distance, "~>12 ft");
    else snprintf(distance, sizeof distance, "~%.1f ft", roundf(feet * 2) * .5f);

    if (strcmp(battery, previousBattery_) == 0 && strcmp(distance, previousDistance_) == 0) return;
    strcpy(previousBattery_, battery);
    strcpy(previousDistance_, distance);
    statusCanvas_.fillScreen(0);
    statusCanvas_.setTextWrap(false);
    statusCanvas_.setTextSize(2);
    statusCanvas_.setTextColor(0xffff);
    statusCanvas_.setCursor(8, 1);
    statusCanvas_.print(battery);
    int16_t x1, y1;
    uint16_t w, h;
    statusCanvas_.getTextBounds(distance, 0, 1, &x1, &y1, &w, &h);
    statusCanvas_.setCursor(width - 8 - w - x1, 1);
    statusCanvas_.print(distance);
    writeCanvas(statusCanvas_, statusY);
  }

 private:
  // Battery Faces uses an ST7789 135x240, the TFT/I2C rail and a page-packed
  // 128x64 OpenToy face converted to RGB565. Use the same panel in landscape.
  static constexpr int width = 240;
  static constexpr int height = 135;
  static constexpr int panelX = 40;
  static constexpr int panelY = 52;
  static constexpr int faceWidth = 208;
  static constexpr int faceHeight = 88;
  static constexpr int statusY = 89;
  static constexpr int footerY = 108;

  static void command(uint8_t code, const uint8_t *data, size_t size) {
    digitalWrite(TFT_DC, LOW);
    SPI.transfer(code);
    digitalWrite(TFT_DC, HIGH);
    if (size) SPI.writeBytes(data, size);
  }

  static void start() {
    SPI.beginTransaction(SPISettings(24000000, MSBFIRST, SPI_MODE0));
    digitalWrite(TFT_CS, LOW);
  }

  static void finish() {
    digitalWrite(TFT_CS, HIGH);
    SPI.endTransaction();
  }

  static void send(uint8_t code, std::initializer_list<uint8_t> data = {}) {
    start();
    command(code, data.begin(), data.size());
    finish();
  }

  static void window(int x, int y, int w, int h) {
    x += panelX;
    y += panelY;
    const int right = x + w - 1;
    const int bottom = y + h - 1;
    const uint8_t columns[] = {uint8_t(x >> 8), uint8_t(x), uint8_t(right >> 8), uint8_t(right)};
    const uint8_t rows[] = {uint8_t(y >> 8), uint8_t(y), uint8_t(bottom >> 8), uint8_t(bottom)};
    command(0x2a, columns, sizeof columns);
    command(0x2b, rows, sizeof rows);
    command(0x2c, nullptr, 0);
  }

  static void clear() {
    uint8_t row[width * 2] = {};
    start();
    window(0, 0, width, height);
    for (int y = 0; y < height; ++y) SPI.writeBytes(row, sizeof row);
    finish();
  }

  static void drawPixels(const uint8_t *pixels, uint16_t ink) {
    uint8_t row[faceWidth * 2];
    start();
    window((width - faceWidth) / 2, 0, faceWidth, faceHeight);
    for (int y = 0; y < faceHeight; ++y) {
      const int sy = y * 64 / faceHeight;
      const uint8_t *page = pixels + (sy / 8) * 128;
      for (int x = 0; x < faceWidth; ++x) {
        const bool on = (page[x * 128 / faceWidth] >> (sy % 8)) & 1;
        row[x * 2] = on ? ink >> 8 : 0;
        row[x * 2 + 1] = on ? ink & 0xff : 0;
      }
      SPI.writeBytes(row, sizeof row);
    }
    finish();
  }

  static void centerText(GFXcanvas16 &canvas, const char *text, int y, uint16_t color) {
    int16_t x1, y1;
    uint16_t w, h;
    canvas.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
    canvas.setCursor((width - w) / 2 - x1, y);
    canvas.setTextColor(color);
    canvas.print(text);
  }

  static void drawBuildInfo() {
    constexpr int footerHeight = height - footerY;
    GFXcanvas16 footer(width, footerHeight);
    if (!footer.getBuffer()) return;
    footer.fillScreen(0);
    footer.setTextWrap(false);
    footer.setTextSize(2);
    char title[32];
    snprintf(title, sizeof title, "%s %s", firmwareName, firmwareVersion);
    centerText(footer, title, 1, 0xffff);
    char timestamp[20];
    buildTimestamp(timestamp);
    footer.setTextSize(1);
    centerText(footer, timestamp, 19, 0xc638);
    writeCanvas(footer, footerY);
  }

  static void writeCanvas(GFXcanvas16 &canvas, int top) {
    uint8_t row[width * 2];
    start();
    window(0, top, width, canvas.height());
    for (int y = 0; y < canvas.height(); ++y) {
      const uint16_t *source = canvas.getBuffer() + y * width;
      for (int x = 0; x < width; ++x) {
        row[x * 2] = source[x] >> 8;
        row[x * 2 + 1] = source[x] & 0xff;
      }
      SPI.writeBytes(row, sizeof row);
    }
    finish();
  }

  FaceState expression(Mood mood, uint32_t elapsed, uint32_t now) const {
    FaceState pose = restingDoodle();
    switch (mood) {
      case Mood::Sleep:
        pose.left = pose.right = 100;
        pose.browBend = -18;
        pose.mouthBend = -21;
        break;
      case Mood::Sad:
        pose.expression = SORRY;
        pose.mouthBend = green_ ? -14 : -22;
        break;
      case Mood::Smile:
        pose.mouthBend = green_ ? 29 : 39;
        pose.mouthSkew = green_ ? -18 : 9;
        break;
      case Mood::Happy:
        pose.expression = HAPPY;
        pose.mouthSkew = green_ ? -21 : 10;
        pose.jaw = green_ ? 24 : 38;
        break;
      case Mood::Elated:
        pose.expression = HAPPY;
        pose.eyeBend = green_ ? 22 : 34;
        pose.browBend = -22;
        pose.jaw = green_ ? 56 : 72;
        break;
      case Mood::Dance: {
        const uint32_t beat = (elapsed / (green_ ? 280 : 215)) % 4;
        pose.expression = beat == 1 ? SURPRISED : HAPPY;
        pose.mouthSkew = beat & 1 ? (green_ ? -45 : 40) : (green_ ? 22 : -25);
        pose.jaw = beat & 1 ? 87 : 42;
        if (beat == 2) (green_ ? pose.left : pose.right) = 100;
        break;
      }
    }
    if (mood != Mood::Sleep && mood != Mood::Dance) {
      const uint32_t period = green_ ? 5400 : 4100;
      const uint32_t phase = (now + (green_ ? 900 : 0)) % period;
      if (phase < 150) pose.left = pose.right = phase < 75 ? 90 : 65;
    }
    return pose;
  }

  bool green_ = false;
  bool drawn_ = false;
  Mood lastMood_ = Mood::Sad;
  uint32_t changedAt_ = 0;
  uint32_t lastTick_ = 0;
  DoodleAnimator motion_;
  uint8_t previous_[1024] = {};
  GFXcanvas16 statusCanvas_{width, footerY - statusY};
  char previousBattery_[14] = {};
  char previousDistance_[14] = {};
};
