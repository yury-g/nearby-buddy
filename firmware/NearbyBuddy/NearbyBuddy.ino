// SPDX-License-Identifier: MIT
#include <Arduino.h>
#include <BLEDevice.h>
#include <Wire.h>
#include <esp_mac.h>
#include <atomic>
#include "host/ble_gap.h"
#include "host/ble_hs_id.h"
#include "Face.h"
#include "Proximity.h"

namespace {
constexpr uint8_t identities[][6] = {
    {0xb8, 0xf8, 0x62, 0xd5, 0x8b, 0x8c},
    {0xac, 0x27, 0x6e, 0xb1, 0xcb, 0xb0},
};
constexpr const char *names[] = {"Blueface", "Greenface"};
// UUID d6adf192-4f29-4da4-bdb9-7b3d07aa9c59 in BLE byte order.
constexpr uint8_t service[] = {
    0x59, 0x9c, 0xaa, 0x07, 0x3d, 0x7b, 0xb9, 0xbd,
    0xa4, 0x4d, 0x29, 0x4f, 0x92, 0xf1, 0xad, 0xd6,
};
constexpr uint8_t serviceData128 = 0x21;
constexpr uint8_t protocol = 1;
constexpr uint8_t payloadLength = 1 + sizeof service + 1 + sizeof identities[0];
constexpr uint16_t bleUnits(unsigned ms) { return ms * 8 / 5; }
struct Profile { const char *name; unsigned adv, scanInterval, scanWindow; };
constexpr Profile profiles[] = {{"fast", 100, 100, 100}, {"eco", 250, 240, 45}};
struct Sample { uint32_t at; int8_t rssi; };
struct BatteryReading {
  bool valid = false;
  float volts = 0;
  float percent = 0;
  float rate = 0;
};

int role = -1;
int profileIndex = 0;
uint8_t addressType = 0;
int advResult = -1;
int scanResult = -1;
QueueHandle_t samples = nullptr;
std::atomic<uint32_t> reports{0};
std::atomic<uint32_t> dropped{0};
Proximity proximity;
Face face;
bool rawLog = false;
bool demo = false;
uint32_t demoStarted = 0;
uint32_t received = 0;
uint32_t frames = 0;
uint32_t frameMaxUs = 0;
uint32_t lastStatus = 0;
uint32_t lastDisplayStatus = 0;
BatteryReading battery;
int lastRssi = 0;
Mood displayed = Mood::Sad;

bool peerPacket(const uint8_t *data, size_t size) {
  while (size > 1) {
    const uint8_t length = data[0];
    if (!length || length >= size) break;
    if (length == payloadLength && data[1] == serviceData128 &&
        memcmp(data + 2, service, sizeof service) == 0 &&
        data[2 + sizeof service] == protocol &&
        memcmp(data + 3 + sizeof service, identities[1 - role], sizeof identities[0]) == 0) {
      return true;
    }
    data += length + 1;
    size -= length + 1;
  }
  return false;
}

int gapEvent(ble_gap_event *event, void *) {
  if (event->type != BLE_GAP_EVENT_DISC) return 0;
  ++reports;
  const auto &report = event->disc;
  if (report.rssi > 20 || report.rssi < -127 ||
      !peerPacket(report.data, report.length_data)) return 0;
  const Sample sample{millis(), report.rssi};
  if (xQueueSend(samples, &sample, 0) != pdTRUE) ++dropped;
  return 0;
}

void startRadio(int index) {
  profileIndex = index;
  if (ble_gap_disc_active()) ble_gap_disc_cancel();
  if (ble_gap_adv_active()) ble_gap_adv_stop();

  uint8_t advData[4 + payloadLength] = {2, 0x01, 0x06, payloadLength, serviceData128};
  memcpy(advData + 5, service, sizeof service);
  advData[5 + sizeof service] = protocol;
  memcpy(advData + 6 + sizeof service, identities[role], sizeof identities[0]);
  // Array includes the length byte as well as the complete AD field.
  advResult = ble_gap_adv_set_data(advData, 4 + payloadLength);
  const Profile &profile = profiles[index];
  ble_gap_adv_params advertise{};
  advertise.conn_mode = BLE_GAP_CONN_MODE_NON;
  advertise.disc_mode = BLE_GAP_DISC_MODE_GEN;
  advertise.itvl_min = advertise.itvl_max = bleUnits(profile.adv);
  if (!advResult) {
    advResult = ble_gap_adv_start(addressType, nullptr, BLE_HS_FOREVER, &advertise, gapEvent, nullptr);
  }
  ble_gap_disc_params scan{};
  scan.itvl = bleUnits(profile.scanInterval);
  scan.window = bleUnits(profile.scanWindow);
  scan.passive = 1;
  scan.filter_duplicates = 0;
  scanResult = ble_gap_disc(addressType, BLE_HS_FOREVER, &scan, gapEvent, nullptr);
}

bool readGauge(uint8_t reg, uint16_t &value) {
  constexpr uint8_t gauge = 0x36;
  Wire.beginTransmission(gauge);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0 || Wire.requestFrom(gauge, uint8_t(2)) != 2) return false;
  const unsigned high = Wire.read();
  value = high << 8 | Wire.read();
  return true;
}

void refreshBattery() {
  uint16_t v = 0, soc = 0, rate = 0;
  const bool valid = readGauge(0x02, v) && readGauge(0x04, soc);
  if (valid) readGauge(0x16, rate);
  battery = {valid, (v >> 4) * .00125f, soc / 256.0f, int16_t(rate) * .208f};
}

float currentFeet(uint32_t now) {
  return proximity.age(now) < Proximity::staleMs ? estimatedFeet(proximity.filtered()) : -1.0f;
}

void status(uint32_t now) {
  if (!Serial) return;
  const Profile &profile = profiles[profileIndex];
  char builtAt[20];
  buildTimestamp(builtAt);
  Serial.printf("NB1 %s ms=%lu profile=%s adv_ms=%u scan_ms=%u/%u adv=%d scan=%d reports=%lu "
                "peer=%lu drop=%lu rssi=%d filtered=%.1f age=%ld est_ft=%.2f mood=%s frames=%lu max_us=%lu "
                "gauge=%d V=%.3f soc=%.2f pct_hr=%.2f demo=%d version=%s built=%.10sT%s\n",
                names[role], (unsigned long)now, profile.name, profile.adv, profile.scanWindow,
                profile.scanInterval, advResult, scanResult, (unsigned long)reports.load(),
                (unsigned long)received, (unsigned long)dropped.load(), lastRssi,
                proximity.filtered(), long(int32_t(proximity.age(now))), currentFeet(now), moodName(displayed),
                (unsigned long)frames, (unsigned long)frameMaxUs, battery.valid,
                battery.volts, battery.percent, battery.rate, demo, firmwareVersion, builtAt, builtAt + 11);
}

void handleCommand(const char *command, uint32_t now) {
  if (strcmp(command, "status") == 0) status(now);
  else if (strcmp(command, "raw on") == 0) rawLog = true;
  else if (strcmp(command, "raw off") == 0) rawLog = false;
  else if (strcmp(command, "fast") == 0) { startRadio(0); status(now); }
  else if (strcmp(command, "eco") == 0) { startRadio(1); status(now); }
  else if (strcmp(command, "demo") == 0) {
    demo = true;
    demoStarted = now;
    Serial.println("NB1 DEMO synthetic expression tour");
  } else if (strncmp(command, "mark ", 5) == 0) {
    Serial.printf("NB1 MARK ms=%lu %s\n", (unsigned long)now, command + 5);
  }
}

void readCommands(uint32_t now) {
  static char command[48];
  static size_t used = 0;
  static bool overflow = false;
  for (unsigned i = 0; i < 48 && Serial.available(); ++i) {
    const char c = Serial.read();
    if (c == '\r' || c == '\n') {
      if (used && !overflow) {
        command[used] = 0;
        handleCommand(command, now);
      }
      used = 0;
      overflow = false;
    } else if (used < sizeof command - 1) {
      command[used++] = c;
    } else {
      overflow = true;
    }
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(3);
  uint8_t mac[6] = {};
  esp_read_mac(mac, ESP_MAC_EFUSE_FACTORY);
  for (int i = 0; i < 2; ++i) {
    if (memcmp(mac, identities[i], sizeof mac) == 0) role = i;
  }
  if (role < 0) return;
  face.begin(role == 1);
  Wire.begin();
  Wire.setTimeOut(20);
  refreshBattery();
  samples = xQueueCreate(24, sizeof(Sample));
  if (samples && BLEDevice::init(String("Nearby Buddy ") + names[role]) &&
      ble_hs_id_infer_auto(0, &addressType) == 0) {
    startRadio(0);
  }
}

void loop() {
  if (role < 0) {
    if (Serial) Serial.println("NB1 ERROR unrecognized factory identity; BLE disabled");
    delay(1000);
    return;
  }

  Sample sample;
  for (unsigned i = 0; samples && i < 16 && xQueueReceive(samples, &sample, 0) == pdTRUE; ++i) {
    ++received;
    lastRssi = sample.rssi;
    proximity.observe(sample.rssi, sample.at);
    if (rawLog && Serial) {
      Serial.printf("NB1 RSSI n=%lu ms=%lu dbm=%d filtered=%.1f\n",
                    (unsigned long)received, (unsigned long)sample.at, sample.rssi, proximity.filtered());
    }
  }

  const uint32_t now = millis();
  const Mood actual = proximity.update(now);
  if (demo && now - demoStarted >= 18000) demo = false;
  const Mood next = demo ? static_cast<Mood>((now - demoStarted) / 3000) : actual;
  if (next != displayed && Serial) {
    Serial.printf("NB1 STATE ms=%lu %s demo=%d\n", (unsigned long)now, moodName(next), demo);
  }
  displayed = next;
  const uint32_t start = micros();
  if (face.draw(displayed, now)) {
    ++frames;
    const uint32_t duration = micros() - start;
    if (duration > frameMaxUs) frameMaxUs = duration;
  }
  readCommands(now);
  if (now - lastStatus >= 2000) {
    lastStatus = now;
    refreshBattery();
    status(now);
  }
  if (now - lastDisplayStatus >= 250) {
    lastDisplayStatus = now;
    const int percent = battery.valid ? constrain(int(lroundf(battery.percent)), 0, 100) : -1;
    face.drawStatus(percent, currentFeet(now));
  }
  delay(2);
}
