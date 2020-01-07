#pragma once

#include <cstdint>
#include <WString.h>

class Game
{
protected:
  Game();

public:
  virtual ~Game();

  virtual String name() const = 0;

  virtual bool onKey(uint8_t x, uint8_t y, bool rising) = 0;
  virtual uint32_t cellColor(uint8_t x, uint8_t y) const = 0;
  virtual String cellColorHtml(uint8_t x, uint8_t y) const = 0;
};
