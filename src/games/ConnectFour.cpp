#include "games/ConnectFour.hpp"

namespace
{
  // There are 64 (8x8) cells. ConnectFour has seven columns and six rows.
  // The leftmost column and the top and bottom row are blue to represent the frame.
  // They are 0 bits in the "interior" mask.
  // Each byte is a column, the most significant byte is the leftmost column,
  // the least significant is the rightmost.
  // Within each byte, the MSB is the bottom and the LSB the top cell.
  // This has been done so that dropping a chip into a column "falls down" to the
  // first non-zero bit in the associated byte.

  uint64_t constexpr one = 0x0000000000000001ull;
  uint64_t constexpr all = 0xffffffffffffffffull;
  uint64_t constexpr interior = 0x007e7e7e7e7e7e7eull;
  uint64_t constexpr column0 = 0x000000000000007eull;
  uint64_t constexpr upper_half = 0x000e0e0e0e0e0e0eull;
  uint64_t constexpr info[2] = {one, one << 56};

  class ConnectFour
    : public Game
  {
    // field0 tracks which player has placed a chip in a cell.
    // 0 is player "false" and 1 is player "true".
    uint64_t field0 = ~interior;
    // A 1 bit in field1 represents a free cell. The frame is regarded as free.
    uint64_t field1 = all;
    uint64_t winMask = 0;
    bool player = false;

    virtual String name() const override
    {
      return "Connect Four";
    };

    static uint8_t scan(
      uint64_t const playerCells,
      uint64_t cell,
      int8_t const shift,
      uint64_t& mask)
    {
      int count = 0;
      while (cell)
      {
        cell = (shift > 0) ? (cell << shift) : (cell >> -shift);
        cell &= interior;

        if (playerCells & cell)
        {
          ++count;
          mask |= cell;
        }
        else
        {
          return count;
        }
      }

      return 0;
    }

    void scan(uint64_t const playerCells, uint64_t const cell, int8_t const shift)
    {
      uint64_t winMaskNew = 0;

      if (
        (scan(playerCells, cell, -shift, winMaskNew) +
          scan(playerCells, cell, shift, winMaskNew)) >= 3
      )
      {
        winMask |= winMaskNew | cell;
      }
    }

    void clear()
    {
      field0 &= ~interior;
      field1 |= interior;
      winMask = 0;
      player = false;
    }

    bool operator==(const ConnectFour& other) const
    {
      return field0 == other.field0 &&
        field1 == other.field1 &&
        winMask == other.winMask &&
        player == other.player;
    }

    bool operator!=(const ConnectFour& other) const
    {
      return !(*this == other);
    }

    bool play(uint8_t y)
    {
      if (winMask)
      {
        return false;
      }

      if (y == 7)
      {
        return false;
      }

      auto const column = column0 << (8 * y);
      auto const freeCells = field1 & column;
      // The "columns" are filled MSB to LSB, higher bits are "lower down".
      auto const cell = (freeCells ^ (freeCells >> 1)) & freeCells;

      if (cell != 0)
      {
        field1 ^= cell;
        field0 ^= cell * static_cast<uint64_t>(player);

        auto const playerCells = ((all * static_cast<uint64_t>(!player)) ^ field0) & ~field1;

        if (cell & upper_half)
        {
          // check vertical (actually just down)
          auto const downMask = cell * 0xf;

          if ((playerCells & downMask) == downMask)
          {
            winMask = downMask;
          }

          scan(playerCells, cell, 7); // check diagonal 1
          scan(playerCells, cell, 9); // check diagonal 2
        }

        scan(playerCells, cell, 8); // check horizontal
      }

      if (cell && !winMask)
      {
        player ^= true;
      }

      return cell != 0;
    }

    bool update_info(bool flag)
    {
      auto const fieldOld{*this};

      field0 |= info[!flag];
      field1 |= info[!flag];

      if (player)
      {
        field0 |= info[flag];
      }
      else
      {
        field0 &= ~info[flag];
      }
      field1 &= ~info[flag];

      if (winMask)
      {
        if (flag)
        {
          field0 &= ~winMask;
          field1 |= winMask;
        }
        else
        {
          if (player)
          {
            field0 |= winMask;
          }
          else
          {
            field0 &= ~winMask;
          }
          field1 &= ~winMask;
        }
      }

      return *this != fieldOld;
    }

    virtual bool update(unsigned long seconds) override
    {
      return update_info(seconds & 1);
    }

    virtual bool onKey(uint8_t x, uint8_t y, bool isReleased) override
    {
      if (isReleased)
      {
        return false;
      }

      if (x == 7 && y == 7)
      {
        clear();
        return true;
      }
      else
      {
        return play(y);
      }
    }

    virtual uint32_t cellColor(uint8_t x, uint8_t y) const override
    {
      uint64_t const cell = one << (x | (y << 3));
      uint8_t const mode = (!!(field0 & cell)) | (!!(field1 & cell) << 1);

      switch (mode)
      {
      case 0:
        return 0x3f0000; // player 0
      case 1:
        return 0x3f3f00; // player 1
      case 2:
        return 0x000000; // free
      case 3:
      default:
        return 0x00003f; // frame
      }
    }

    virtual String cellColorHtml(uint8_t x, uint8_t y) const override
    {
      uint64_t const cell = one << (x | (y << 3));
      uint8_t const mode = (!!(field0 & cell)) | (!!(field1 & cell) << 1);

      switch (mode)
      {
      case 0:
        return "red"; // player 0
      case 1:
        return "yellow"; // player 1
      case 2:
        return "black"; // free
      case 3:
      default:
        return "blue"; // frame
      }
    }

  public:
    ConnectFour()
    {
    }
  };
} // namespace

std::shared_ptr<Game>
createConnectFour()
{
  return std::make_shared<ConnectFour>();
}
