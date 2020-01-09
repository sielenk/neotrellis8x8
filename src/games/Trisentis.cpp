#include "games/Trisentis.hpp"

namespace
{
uint64_t const one = 1;
uint64_t const seven = 7;

uint8_t join(uint8_t x, uint8_t y)
{
  return x | (y << 3);
}

class Triesentis
    : public Game
{
  uint64_t field = 0;
  uint64_t mask[64];

  virtual String name() const override
  {
    return "Trisentis";
  };

  virtual bool onKey(uint8_t x, uint8_t y, bool isReleased) override
  {
    if (isReleased)
    {
      return false;
    }

    field ^= mask[join(x, y)];

    return true;
  }

  virtual uint32_t cellColor(uint8_t x, uint8_t y) const override
  {
    return (field & (one << join(x, y))) ? 0x003f00 : 0x3f0000;
  }

  virtual String cellColorHtml(uint8_t x, uint8_t y) const override
  {
    return (field & (one << join(x, y))) ? "green" : "red";
  }

public:
  Triesentis()
  {
    for (uint8_t i = 0; i < 64; ++i)
    {
      uint8_t const x = i & 7;
      uint8_t const y = i >> 3;
      uint64_t const rowMask = (((seven << x) >> 1) & 0xff) << (8 * y);
      mask[i] = (rowMask | (rowMask << 8) | (rowMask >> 8)) ^ (one << i);
    }
  }
};
} // namespace

std::shared_ptr<Game> createTrisentis()
{
  return std::make_shared<Triesentis>();
}
