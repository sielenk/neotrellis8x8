#pragma once

#include <cstdint>
#include <WString.h>

class Game
{
  enum CellState : uint8_t
  {
    EMPTY,
    PLAYER1,
    PLAYER2,
    GRID = UINT8_MAX
  };

  CellState field[3][3];
  CellState player;
  CellState winner;

  void init();

  CellState *cell(uint8_t x, uint8_t y);

  CellState const *cell(uint8_t x, uint8_t y) const
  {
    return const_cast<Game *>(this)->cell(x, y);
  }

  CellState cellState(uint8_t x, uint8_t y) const;

public:
  Game();

  String name() const;

  bool onKey(uint8_t x, uint8_t y, bool rising);
  uint32_t cellColor(uint8_t x, uint8_t y) const;
  String cellColorHtml(uint8_t x, uint8_t y) const;
};
