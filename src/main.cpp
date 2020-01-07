#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <FS.h>
#include <Adafruit_NeoTrellis.h>
#include <Schedule.h>

#include <memory>

#define INT_PIN 14
#define ANALOG_PIN A0

Adafruit_NeoTrellis trelli[4] = {
    Adafruit_NeoTrellis{NEO_TRELLIS_ADDR},
    Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 1},
    Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 2},
    Adafruit_NeoTrellis{NEO_TRELLIS_ADDR + 3}};

auto trellis = Adafruit_MultiTrellis{trelli, 2, 2};

WiFiManager wifiManager;
ESP8266WebServer server;

void setAll(uint32_t color)
{
  for (uint8_t i = 0; i < 64; i++)
  {
    trellis.setPixelColor(i, color);
    trellis.show();
    delay(5);
  }
}

class Game
{
  static uint8_t map(uint8_t xy)
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

  void init()
  {
    bzero(field, sizeof(field));
    player = PLAYER1;
    winner = EMPTY;
  }

  CellState *cell(uint8_t x, uint8_t y)
  {
    auto const xx = map(x);
    auto const yy = map(y);

    if (xx == UINT8_MAX || yy == UINT8_MAX)
    {
      return nullptr;
    }

    return &field[xx][yy];
  }

  CellState const *cell(uint8_t x, uint8_t y) const
  {
    return const_cast<Game *>(this)->cell(x, y);
  }

  CellState cellState(uint8_t x, uint8_t y) const
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

public:
  Game()
  {
    init();
  }

  bool onKey(uint8_t x, uint8_t y, bool rising)
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

  uint32_t cellColor(uint8_t x, uint8_t y) const
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
  cellColorHtml(uint8_t x, uint8_t y) const
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
};

Game game;

void showGame()
{
  for (uint8_t x = 0; x < 8; ++x)
  {
    for (uint8_t y = 0; y < 8; ++y)
    {
      auto const pixelColor = game.cellColor(x, y);

      trellis.setPixelColor(x, y, pixelColor);
    }
  }

  trellis.show();
}

TrellisCallback onKey(keyEvent evt)
{
  bool isRising;
  switch (evt.bit.EDGE)
  {
  case SEESAW_KEYPAD_EDGE_FALLING:
    isRising = false;
    break;
  case SEESAW_KEYPAD_EDGE_RISING:
    isRising = false;
    break;
  default:
    return nullptr;
  }

  if (game.onKey(evt.bit.NUM & 7, (evt.bit.NUM >> 3) & 7, isRising))
  {
    showGame();
  }

  return nullptr;
}

double getBatteryVoltage()
{
  // adjusted according to measured values on my device
  return (analogRead(ANALOG_PIN) * 3.87) / 834.0;
}

void handlerRoot()
{
  String buffer;

  buffer += "<!DOCTYPE html><html><head><meta http-equiv='Content-Type' content='text/html; charset=utf-8'></head><body><table border='1'>";
  for (uint8_t y = 0; y < 8; ++y)
  {
    buffer += "<tr>";
    for (uint8_t x = 0; x < 8; ++x)
    {
      buffer += "<td style='width:20px; height:20px; background-color: ";
      buffer += game.cellColorHtml(x, y);
      buffer += ";'/>";
    }
    buffer += "</tr>";
  }
  buffer += "</table><p>Battery voltage: ";
  buffer += getBatteryVoltage();
  buffer += "V</p></body></html>";

  server.send(200, "text/html", buffer.c_str(), buffer.length());
}

void handlerIcon()
{
  auto file = SPIFFS.open("/favicon.ico", "r");
  auto const size = file.size();
  auto const buffer = std::unique_ptr<char>{new char[size]};

  file.readBytes(buffer.get(), size);
  server.send(200, "image/x-icon", buffer.get(), size);

  file.close();
}

void setup()
{
  Serial.begin(74880);
  Serial.println("setup - start");

  pinMode(INT_PIN, INPUT);

  Serial.printf("trellis: %s\r\n", trellis.begin() ? "succeeded" : "failed");

  setAll(0x00004f);

  wifiManager.autoConnect();

  setAll(0x004f00);

  Serial.printf("SPIFFS: %s\r\n", SPIFFS.begin() ? "succeeded" : "failed");

  setAll(0x4f0000);

  server.on("/", HTTP_GET, handlerRoot);
  server.on("/favicon.ico", HTTP_GET, handlerIcon);
  server.begin();
  Serial.println("HTTP server started");

  setAll(0x000000);

  for (uint8_t i = 0; i < 64; ++i)
  {
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.registerCallback(i, &onKey);
  }

  Serial.println("setup - end");

  showGame();
}

unsigned long secondsOld;

void loop()
{
  if (digitalRead(INT_PIN) == 0)
  {
    trellis.read();
  }

  unsigned long const secondsNew = millis() / 1000;

  if (secondsOld != secondsNew)
  {
    auto const val = getBatteryVoltage();
    Serial.printf("%lu\t%05.3fV\r\n", secondsNew, val);

    secondsOld = secondsNew;
  }

  server.handleClient();

  delay(2);
}
