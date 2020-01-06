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
    return -1;
  }
}

uint8_t unmap(uint8_t xy)
{
  return 3 * xy;
}

uint32_t playerColor(int8_t player, int8_t winner)
{
  switch (player)
  {
  case 1:
    return (winner == 2) ? 0x0f0000 : 0x4f0000;
  case 2:
    return (winner == 1) ? 0x000f00 : 0x004f00;
  default:
    return 0x000000;
  }
}

int8_t field[3][3] = {};
int8_t player = 1;
int8_t winner = -1;

TrellisCallback onKey(keyEvent evt)
{
  if (winner != -1)
  {
    bzero(field, sizeof(field));
    player = 1;
    winner = -1;
  }
  else
  {
    auto const xx = map(evt.bit.NUM & 7);
    auto const yy = map((evt.bit.NUM >> 3) & 7);
    auto &cell = field[xx][yy];

    if (xx != -1 && yy != -1 && !cell)
    {
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
        winner = 1;
      }
      else if (rp[0] == 8 || rp[1] == 8 || rp[2] == 8 || cp[0] == 8 || cp[1] == 8 || cp[2] == 8 || dp0 == 8 || dp1 == 8)
      {
        // win player 2
        winner = 2;
      }
      else if (rp[0] && rp[1] && rp[2])
      {
        // draw
        winner = 0;
      }
      else
      {
        player ^= 3;
      }
    }
  }

  for (uint8_t xx = 0; xx < 3; ++xx)
  {
    for (uint8_t yy = 0; yy < 3; ++yy)
    {
      auto const pixelColor = playerColor(field[xx][yy], winner);
      auto const x = unmap(xx);
      auto const y = unmap(yy);

      trellis.setPixelColor(x, y, pixelColor);
      trellis.setPixelColor(x + 1, y, pixelColor);
      trellis.setPixelColor(x, y + 1, pixelColor);
      trellis.setPixelColor(x + 1, y + 1, pixelColor);
    }
  }
  trellis.show();

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
  for (uint8_t y = 0; y < 3; ++y)
  {
    buffer += "<tr>";
    for (uint8_t x = 0; x < 3; ++x)
    {
      auto const c(field[x][y]);

      buffer += "<td style='width:20px; height:20px; background-color: ";
      switch (c)
      {
      case 1:
        buffer += "red";
        break;
      case 2:
        buffer += "green";
        break;
      default:
        buffer += "black";
        break;
      }
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

  Serial.printf("size = %d\r\n", size);

  file.readBytes(buffer.get(), size);
  server.send(200, "image/x-icon", buffer.get(), size);

  file.close();
}

void ICACHE_RAM_ATTR handlerKeyInterrupt()
{
  schedule_function([]() {
    trellis.read();
  });
}

void setup()
{
  Serial.begin(74880);
  Serial.println("setup - start");

  wifiManager.autoConnect();

  pinMode(INT_PIN, INPUT);
  attachInterrupt(INT_PIN, &handlerKeyInterrupt, FALLING);

  Serial.printf("trellis: %s\r\n", trellis.begin() ? "succeeded" : "failed");
  Serial.printf("SPIFFS: %s\r\n", SPIFFS.begin() ? "succeeded" : "failed");

  setAll(0x00004f);
  setAll(0x004f00);
  setAll(0x4f0000);
  setAll(0x000000);

  for (int i = 0; i < 8; ++i)
  {
    trellis.setPixelColor(i, 2, 0x4f4f4f);
    trellis.setPixelColor(i, 5, 0x4f4f4f);
    trellis.setPixelColor(2, i, 0x4f4f4f);
    trellis.setPixelColor(5, i, 0x4f4f4f);
  }
  trellis.show();

  for (uint8_t i = 0; i < 64; ++i)
  {
    trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
    trellis.registerCallback(i, &onKey);
  }

  server.on("/", HTTP_GET, handlerRoot);
  server.on("/favicon.ico", HTTP_GET, handlerIcon);
  server.begin();
  Serial.println("HTTP server started");

  Serial.println("setup - end");
}

unsigned long secondsOld;

void loop()
{
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
