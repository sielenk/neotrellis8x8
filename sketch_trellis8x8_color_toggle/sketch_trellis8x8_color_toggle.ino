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

uint8_t colors[64] = { };

TrellisCallback onKey(keyEvent evt) {
  uint8_t& color = colors[evt.bit.NUM];
  ++color;

  uint32_t const pixelColor =
    (((color & 1) != 0) * 0x7f0000) |
    (((color & 2) != 0) * 0x007f00) |
    (((color & 4) != 0) * 0x00007f) |
    0;

  trellis.setPixelColor(evt.bit.NUM, pixelColor); //on rising
  trellis.show();

  return nullptr;
}

#define INT_PIN 13

void setup() {
  pinMode(INT_PIN, INPUT);

  trellis.begin();

  setAll(0x00007f);
  setAll(0x007f00);
  setAll(0x7f0000);
  setAll(0x000000);

  for (uint8_t i = 0; i < 64; ++i) {
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.registerCallback(i, &onKey);
  }
}

void loop() {
  if (!digitalRead(INT_PIN)) {
    trellis.read();
  }

  delay(2);
}
