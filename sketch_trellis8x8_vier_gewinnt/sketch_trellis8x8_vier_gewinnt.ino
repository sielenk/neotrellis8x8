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
uint64_t const info[2] = { one, one << 56 };

class Field {
    static uint64_t const interior   = 0x007e7e7e7e7e7e7eull;
    static uint64_t const upper_half = 0x000e0e0e0e0e0e0eull;

    uint64_t field0  = ~interior;
    uint64_t field1  = 0xffffffffffffffffull;
    uint64_t winMask = 0;

    uint8_t scan(uint64_t const playerCells, uint64_t cell, int8_t const shift, uint64_t& mask) {
      int count = 0;
      while (cell) {
        cell = (shift > 0) ? (cell << shift) : (cell >> -shift);
        cell &= interior;

        if (playerCells & cell) {
          ++count;
          mask |= cell;
        } else {
          return count;
        }
      }

      return 0;
    }

    void scan(uint64_t const playerCells, uint64_t const cell, int8_t const shift) {
      uint64_t winMaskNew = 0;

      if (scan(playerCells, cell, -shift, winMaskNew) + scan(playerCells, cell, shift, winMaskNew) >= 3) {
        winMask |= winMaskNew | cell;
      }
    }

  public:
    bool operator==(const Field& other) const {
      return field0 == other.field0 && field1 == other.field1;
    }

    bool operator!=(const Field& other) const {
      return !(*this == other);
    }

    uint32_t get_color(uint8_t index) const {
      uint64_t const fieldMask = one << index;
      uint8_t  const field = ((field0 & fieldMask) != 0) | (((field1 & fieldMask) != 0) << 1);

      switch (field) {
        case 0: return 0x4f0000; // player 0
        case 1: return 0x4f4f00; // player 1
        case 2: return 0x000000; // free
        case 3: return 0x00004f; // frame
      }
    }

    void clear() {
      field0 &= ~interior;
      field1 |=  interior;
      winMask = 0;
    }

    bool play(uint8_t index, bool player) {
      if (winMask) {
        return false;
      }

      auto const y = (index >> 3) & 7;

      if (y == 7) {
        return false;
      }

      auto const column    = static_cast<uint64_t>(0x7e) << (8 * y);
      auto const freeCells = field1 & column;
      auto const cell      = (freeCells ^ (freeCells >> 1)) & freeCells;

      if (cell != 0) {
        field1 ^= cell;
        if (player) {
          field0 ^= cell;
        }

        auto const playerCells = (player ? field0 : ~field0) & ~field1;

        if (cell & upper_half) { // check vertical (actually just down)
          auto const downMask = cell * 0xf;

          if ((playerCells & downMask) == downMask) {
            winMask = downMask;
          }

          scan(playerCells, cell, 7); // check diagonal 1
          scan(playerCells, cell, 9); // check diagonal 2
        }

        scan(playerCells, cell, 8); // check horizontal
      }

      return cell != 0;
    }

    bool update_info(bool flag, bool player) {
      auto const fieldOld(*this);

      field0 |= info[!flag];
      field1 |= info[!flag];

      if (player) {
        field0 |= info[flag];
      } else {
        field0 &= ~info[flag];
      }
      field1 &= ~info[flag];

      if (winMask) {
        if (flag) {
          field0 &= ~winMask;
          field1 |= winMask;
        } else {
          if (player) {
            field0 &= ~winMask;
          } else {
            field0 |= winMask;
          }
          field1 &= ~winMask;
        }
      }

      return *this != fieldOld;
    }
} field;

void show_field() {
  for (uint8_t i = 0; i < 64; ++i) {
    trellis.setPixelColor(i, field.get_color(i));
  }
  trellis.show();
}

bool player = false;

TrellisCallback onKey(keyEvent evt) {
  if (evt.bit.NUM == 63) {
    field.clear();
    show_field();
  } else if (field.play(evt.bit.NUM, player)) {
    player ^= true;
    show_field();
  }

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

  show_field();
}

void loop() {
  if (!digitalRead(INT_PIN)) {
    trellis.read();
  }

  auto const pulse(millis() % 1000 < 500);

  if (field.update_info(pulse, player)) {
    show_field();
  }

  delay(2);
}
