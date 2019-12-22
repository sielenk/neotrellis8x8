#include <Adafruit_NeoTrellis.h>

Adafruit_NeoTrellis trelli[4] = {
  Adafruit_NeoTrellis{NEO_TRELLIS_ADDR},
  Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 1},
  Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 2},
  Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 3}
};

auto trellis = Adafruit_MultiTrellis{trelli, 2, 2};

void setAll(uint32_t color) {
  for (uint8_t i = 0; i < 64; i++) {
    trellis.setPixelColor(i, color);
    trellis.show();
    delay(5);
  }
}

uint64_t const one = 1;
uint64_t const seven = 7;
uint64_t field = 0;
uint64_t mask[64];

void show_field() {
  for (uint8_t i = 0; i < 64; ++i) {
    trellis.setPixelColor(i, (field & (one << i)) ? 0x007f00 : 0x7f0000);
  }
  trellis.show();
}

TrellisCallback onKey(keyEvent evt) {
  field ^= mask[evt.bit.NUM];
  show_field();
  return nullptr;
}

#define INT_PIN 14

void setup() {
  Serial.begin(74880);
  pinMode(INT_PIN, INPUT);
  trellis.begin();

  setAll(0x00004f);
  setAll(0x004f00);
  setAll(0x4f0000);
  setAll(0x000000);

  for (uint8_t i = 0; i < 64; ++i) {
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.registerCallback(i, &onKey);

    uint8_t const x = i & 7;
    uint8_t const y = i >> 3;
    uint64_t const rowMask = (((seven << x) >> 1) & 0xff) << (8 * y);
    mask[i] = (rowMask | (rowMask << 8) | (rowMask >> 8)) ^ (one << i);
  }

  show_field();
}

void loop() {
  if (!digitalRead(INT_PIN)) {
    trellis.read();
  }

  delay(2);
}
