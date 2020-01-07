#include "ColorToggle.hpp"

class ColorToggle
    : public Game
{
  uint8_t field[8][8];

  virtual String name() const override
  {
    return "Color Toggle";
  };

  virtual bool onKey(uint8_t x, uint8_t y, bool isReleased)
  {
    if (isReleased)
    {
      return false;
    }

    auto &cell(field[x][y]);

    cell = (cell + 1) & 7;

    return true;
  }

  virtual uint32_t cellColor(uint8_t x, uint8_t y) const
  {
    auto const cell(field[x][y]);

    return (((cell & 1) != 0) * 0x4f0000) |
           (((cell & 2) != 0) * 0x004f00) |
           (((cell & 4) != 0) * 0x00004f) |
           0;
  }

  virtual String cellColorHtml(uint8_t x, uint8_t y) const
  {
    auto const cell(field[x][y]);

    auto const color = (((cell & 1) != 0) * 0xff0000) |
                       (((cell & 2) != 0) * 0x00ff00) |
                       (((cell & 4) != 0) * 0x0000ff) |
                       0;

    char buffer[16];
    ets_sprintf(buffer, "#%06x", color);
    return buffer;
  }
};

std::shared_ptr<Game> createColorToggle()
{
  return std::make_shared<ColorToggle>();
}
