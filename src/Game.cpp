#include "Game.hpp"

namespace
{
uint8_t map(uint8_t xy)
{
  switch (xy)
  {
  case 0:
  case 1:
    return 0;
  case 3:
  case 4:
    return 1;
  case 6:
  case 7:
    return 2;
  default:
    return UINT8_MAX;
  }
}
} // namespace

Game::Game()
{
  init();
}

String Game::name() const
{
  return "Tic Tac Toe";
}

void Game::init()
{
  bzero(field, sizeof(field));
  player = PLAYER1;
  winner = EMPTY;
}

auto Game::cell(uint8_t x, uint8_t y) -> CellState *
{
  auto const xx = map(x);
  auto const yy = map(y);

  if (xx == UINT8_MAX || yy == UINT8_MAX)
  {
    return nullptr;
  }

  return &field[xx][yy];
}

auto Game::cellState(uint8_t x, uint8_t y) const -> CellState
{
  if (CellState const *cellPtr = cell(x, y))
  {
    return *cellPtr;
  }
  else
  {
    return GRID;
  }
}

bool Game::onKey(uint8_t x, uint8_t y, bool rising)
{
  if (winner != EMPTY)
  {
    init();
  }
  else
  {
    CellState *const cellPtr = cell(x, y);

    if (!cellPtr || *cellPtr != EMPTY)
    {
      return false;
    }

    auto &cell = *cellPtr;

    cell = player;

    uint8_t rp[] = {1, 1, 1};
    uint8_t cp[] = {1, 1, 1};
    uint8_t dp0 = 1;
    uint8_t dp1 = 1;

    for (uint8_t u = 0; u < 3; ++u)
    {
      dp0 *= field[u][u];
      dp1 *= field[u][2 - u];

      for (uint8_t v = 0; v < 3; ++v)
      {
        rp[u] *= field[u][v];
        cp[u] *= field[v][u];
      }
    }

    if (rp[0] == 1 || rp[1] == 1 || rp[2] == 1 || cp[0] == 1 || cp[1] == 1 || cp[2] == 1 || dp0 == 1 || dp1 == 1)
    {
      // win player 1
      winner = PLAYER1;
    }
    else if (rp[0] == 8 || rp[1] == 8 || rp[2] == 8 || cp[0] == 8 || cp[1] == 8 || cp[2] == 8 || dp0 == 8 || dp1 == 8)
    {
      // win player 2
      winner = PLAYER2;
    }
    else if (rp[0] && rp[1] && rp[2])
    {
      // draw
      winner = GRID;
    }
    else
    {
      player = (player == PLAYER1) ? PLAYER2 : PLAYER1;
    }
  }

  return true;
}

uint32_t Game::cellColor(uint8_t x, uint8_t y) const
{
  switch (cellState(x, y))
  {
  case PLAYER1:
    return (winner == PLAYER2) ? 0x0f0000 : 0x3f0000;
  case PLAYER2:
    return (winner == PLAYER1) ? 0x000f00 : 0x003f00;
  case GRID:
    return (winner != EMPTY) ? 0x3f3f3f : (player == PLAYER1) ? 0x3f2a2a : 0x2a3f2a;
  default:
    return 0x000000;
  }
}

String
Game::cellColorHtml(uint8_t x, uint8_t y) const
{
  switch (cellState(x, y))
  {
  case PLAYER1:
    return (winner == PLAYER2) ? "darkred" : "red";
  case PLAYER2:
    return (winner == PLAYER1) ? "green" : "lightgreen";
  case GRID:
    return (winner != EMPTY) ? "white" : (player == PLAYER1) ? "#ffe0e0" : "#e0ffe0";
  default:
    return "black";
  }
}
