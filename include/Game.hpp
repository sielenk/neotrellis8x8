#pragma once

#include <WString.h>

class Game
{
protected:
  Game();

public:
  virtual ~Game();

  [[nodiscard]] virtual String name() const = 0;

  virtual bool update(unsigned long seconds);
  virtual bool onKey(uint8_t x, uint8_t y, bool isReleased) = 0;
  [[nodiscard]] virtual uint32_t cellColor(uint8_t x, uint8_t y) const = 0;
  [[nodiscard]] virtual String cellColorHtml(uint8_t x, uint8_t y) const = 0;
};
